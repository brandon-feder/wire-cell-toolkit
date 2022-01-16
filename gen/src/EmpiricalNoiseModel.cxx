
#include "WireCellGen/EmpiricalNoiseModel.h"

#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/Response.h"  // fixme: should remove direct dependency

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/FFTBestLength.h"

#include <iostream>  // debug

WIRECELL_FACTORY(EmpiricalNoiseModel,
                 WireCell::Gen::EmpiricalNoiseModel,
                 WireCell::INamed,
                 WireCell::IChannelSpectrum,
                 WireCell::IConfigurable)

using namespace WireCell;

Gen::EmpiricalNoiseModel::EmpiricalNoiseModel(const std::string& spectra_file, const int nsamples, const double period,
                                              const double wire_length_scale,
                                              // const double time_scale,
                                              // const double gain_scale,
                                              // const double freq_scale,
                                              const std::string anode_tn, const std::string chanstat_tn)
  : Aux::Logger("EmpericalNoiseModel", "gen")
  , m_spectra_file(spectra_file)
  , m_nsamples(nsamples)
  , m_period(period)
  , m_wlres(wire_length_scale)
  // , m_tres(time_scale)
  // , m_gres(gain_scale)
  // , m_fres(freq_scale)
  , m_anode_tn(anode_tn)
  , m_chanstat_tn(chanstat_tn)
  , m_amp_cache(4)
{
    m_fft_length = fft_best_length(m_nsamples);
    //  m_fft_length = m_nsamples;
    gen_elec_resp_default();
}

Gen::EmpiricalNoiseModel::~EmpiricalNoiseModel() {}

void Gen::EmpiricalNoiseModel::gen_elec_resp_default()
{
    // double shaping[5]={1,1.1,2,2.2,3}; // us

    // calculate the frequencies ...
    m_elec_resp_freq.resize(m_fft_length, 0);
    for (unsigned int i = 0; i != m_elec_resp_freq.size(); i++) {
        if (i <= m_elec_resp_freq.size() / 2.) {
            m_elec_resp_freq.at(i) =
                i / (m_elec_resp_freq.size() * 1.0) * 1. / m_period;  // the second half is useless ...
        }
        else {
            m_elec_resp_freq.at(i) = (m_elec_resp_freq.size() - i) / (m_elec_resp_freq.size() * 1.0) * 1. /
                                     m_period;  // the second half is useless ...
        }

        // if (m_elec_resp_freq.at(i) > 1./m_period / 2.){
        //   m_elec_resp_freq.resize(i);
        //   break;
        // }
    }

    // for (int i=0;i!=5;i++){
    //   Response::ColdElec elec_resp(1, shaping[i]); // default at 1 mV/fC
    //   auto sig   =   elec_resp.generate(WireCell::Waveform::Domain(0, m_fft_length*m_period), m_fft_length);
    //   auto filt   = Waveform::dft(sig);
    //   int nconfig = shaping[i]/ 0.1;
    //   auto ele_resp_amp = Waveform::magnitude(filt);
    //   m_elec_resp_cache[nconfig] = ele_resp_amp;
    // }
}

WireCell::Configuration Gen::EmpiricalNoiseModel::default_configuration() const
{
    Configuration cfg;

    /// The file holding the spectral data
    cfg["spectra_file"] = m_spectra_file;
    cfg["nsamples"] = m_nsamples;  // number of samples up to Nyquist frequency
    cfg["period"] = m_period;
    cfg["wire_length_scale"] = m_wlres;  //
    // cfg["time_scale"] = m_tres;
    // cfg["gain_scale"] = m_gres;
    // cfg["freq_scale"] = m_fres;
    cfg["anode"] = m_anode_tn;  // name of IAnodePlane component

    return cfg;
}

void Gen::EmpiricalNoiseModel::resample(NoiseSpectrum& spectrum) const
{
    if (spectrum.nsamples == m_fft_length && spectrum.period == m_period) {
        return;
    }

    // scale the amplitude ...
    double scale = sqrt(m_fft_length / (spectrum.nsamples * 1.0) * spectrum.period / (m_period * 1.0));
    spectrum.constant *= scale;
    for (unsigned int ind = 0; ind != spectrum.amps.size(); ind++) {
        spectrum.amps[ind] *= scale;
    }

    // interpolate ...

    amplitude_t temp_amplitudes(m_fft_length, 0);
    int count_low = 0;
    int count_high = 1;
    double mu = 0;
    for (int i = 0; i != m_fft_length; i++) {
        double frequency = m_elec_resp_freq.at(i);
        if (frequency <= spectrum.freqs[0]) {
            count_low = 0;
            count_high = 1;
            mu = 0;
        }
        else if (frequency >= spectrum.freqs.back()) {
            count_low = spectrum.freqs.size() - 2;
            count_high = spectrum.freqs.size() - 1;
            mu = 1;
        }
        else {
            for (unsigned int j = 0; j != spectrum.freqs.size(); j++) {
                if (frequency > spectrum.freqs.at(j)) {
                    count_low = j;
                    count_high = j + 1;
                }
                else {
                    break;
                }
            }
            mu = (frequency - spectrum.freqs.at(count_low)) /
                 (spectrum.freqs.at(count_high) - spectrum.freqs.at(count_low));
        }

        temp_amplitudes.at(i) = (1 - mu) * spectrum.amps[count_low] + mu * spectrum.amps[count_high];
    }

    spectrum.amps = temp_amplitudes;
    spectrum.amps.push_back(spectrum.constant);

    return;
}

void Gen::EmpiricalNoiseModel::configure(const WireCell::Configuration& cfg)
{
    m_anode_tn = get(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);

    m_chanstat_tn = get(cfg, "chanstat", m_chanstat_tn);
    if (m_chanstat_tn != "") {  // allow for an empty channel status, no deviation from norm
        m_chanstat = Factory::find_tn<IChannelStatus>(m_chanstat_tn);
    }

    log->debug("using anode {}, chanstat {}", m_anode_tn, m_chanstat_tn);

    m_spectra_file = get(cfg, "spectra_file", m_spectra_file);

    m_nsamples = get(cfg, "nsamples", m_nsamples);
    m_fft_length = fft_best_length(m_nsamples);
    // m_fft_length = m_nsamples;
    m_period = get(cfg, "period", m_period);
    m_wlres = get(cfg, "wire_length_scale", m_wlres);
    // m_tres = get(cfg, "time_scale", m_tres);
    // m_gres = get(cfg, "gain_scale", m_gres);
    // m_fres = get(cfg, "freq_scale", m_fres);

    // Load the data file assuming.  The file should be a list of
    // dictionaries matching the
    // wirecell.sigproc.noise.schema.NoiseSpectrum class.  Fixme:
    // should break out this code separate.
    auto jdat = Persist::load(m_spectra_file);
    const int nentries = jdat.size();
    m_spectral_data.clear();

    gen_elec_resp_default();

    for (int ientry = 0; ientry < nentries; ++ientry) {
        auto jentry = jdat[ientry];
        NoiseSpectrum* nsptr = new NoiseSpectrum();

        nsptr->plane = jentry["plane"].asInt();
        nsptr->nsamples = jentry["nsamples"].asInt();
        nsptr->period = jentry["period"].asFloat();    // * m_tres;
        nsptr->gain = jentry["gain"].asFloat();        // *m_gres;
        nsptr->shaping = jentry["shaping"].asFloat();  // * m_tres;
        nsptr->wirelen = jentry["wirelen"].asFloat();  // * m_wlres;
        nsptr->constant = jentry["const"].asFloat();

        auto jfreqs = jentry["freqs"];
        const int nfreqs = jfreqs.size();
        nsptr->freqs.resize(nfreqs, 0.0);
        for (int ind = 0; ind < nfreqs; ++ind) {
            nsptr->freqs[ind] = jfreqs[ind].asFloat();  // * m_fres;
        }
        auto jamps = jentry["amps"];
        const int namps = jamps.size();
        nsptr->amps.resize(namps + 1, 0.0);
        for (int ind = 0; ind < namps; ++ind) {
            nsptr->amps[ind] = jamps[ind].asFloat();
        }
        // put the constant term at the end of the amplitude ...
        // nsptr->amps[namps] = nsptr->constant;

        resample(*nsptr);
        m_spectral_data[nsptr->plane].push_back(nsptr);  // assumes ordered by wire length!
    }
}

IChannelSpectrum::amplitude_t Gen::EmpiricalNoiseModel::interpolate(int iplane, double wire_length) const
{
    auto it = m_spectral_data.find(iplane);
    if (it == m_spectral_data.end()) {
        return amplitude_t();
    }

    const auto& spectra = it->second;

    // Linearly interpolate spectral data to wire length.

    // Any wire lengths
    // which are out of bounds causes the nearest result to be
    // returned (flat extrapolation)
    const NoiseSpectrum* front = spectra.front();
    if (wire_length <= front->wirelen) {
        return front->amps;
    }
    const NoiseSpectrum* back = spectra.back();
    if (wire_length >= back->wirelen) {
        return back->amps;
    }

    const int nspectra = spectra.size();
    for (int ind = 1; ind < nspectra; ++ind) {
        const NoiseSpectrum* hi = spectra[ind];
        if (hi->wirelen < wire_length) {
            continue;
        }
        const NoiseSpectrum* lo = spectra[ind - 1];

        const double delta = hi->wirelen - lo->wirelen;
        const double dist = wire_length - lo->wirelen;
        const double mu = dist / delta;

        const int nsamp = lo->amps.size();
        amplitude_t amp(nsamp);
        for (int isamp = 0; isamp < nsamp; ++isamp) {  // regular amplitude ...
            amp[isamp] = lo->amps[isamp] * (1 - mu) + hi->amps[isamp] * mu;
        }
        // amp[nsamp] = lo->constant*(1-mu) + hi->constant*mu; // do the constant term ...
        return amp;
    }
    // should not get here
    return amplitude_t();
}

const std::vector<float>& Gen::EmpiricalNoiseModel::freq() const
{
    // assume frequency is universal ...
    const int iplane = 0;
    auto it = m_spectral_data.find(iplane);
    const auto& spectra = it->second;
    return spectra.front()->freqs;
}

const double Gen::EmpiricalNoiseModel::shaping_time(int chid) const
{
    auto wpid = m_anode->resolve(chid);
    const int iplane = wpid.index();
    auto it = m_spectral_data.find(iplane);
    const auto& spectra = it->second;
    return spectra.front()->shaping;
}

const double Gen::EmpiricalNoiseModel::gain(int chid) const
{
    auto wpid = m_anode->resolve(chid);
    const int iplane = wpid.index();
    auto it = m_spectral_data.find(iplane);
    const auto& spectra = it->second;
    return spectra.front()->gain;
}

const IChannelSpectrum::amplitude_t& Gen::EmpiricalNoiseModel::operator()(int chid) const
{
    // get truncated wire length for cache
    int ilen = 0;
    auto chlen = m_chid_to_intlen.find(chid);
    if (chlen == m_chid_to_intlen.end()) {  // new channel
        auto wires = m_anode->wires(chid);  // sum up wire length
        double len = 0.0;
        for (auto wire : wires) {
            len += ray_length(wire->ray());
        }
        // cache every cm ...
        ilen = int(len / m_wlres);
        // there might be  aproblem with the wire length's unit
        // ilen = int(len);
        m_chid_to_intlen[chid] = ilen; // fixme: not thread safe
    }
    else {
        ilen = chlen->second;
    }

    // saved content
    auto wpid = m_anode->resolve(chid);
    const int iplane = wpid.index();

    auto& amp_cache = m_amp_cache.at(iplane);
    auto lenamp = amp_cache.find(ilen);
    if (lenamp == amp_cache.end()) {
        auto amp = interpolate(iplane, ilen * m_wlres);  // interpolate ...
        amp_cache[ilen] = amp;  // fixme: not thread safe
    }
    lenamp = amp_cache.find(ilen);

    // prepare to scale ...
    double db_gain = gain(chid);
    double db_shaping = shaping_time(chid);

    // get channel gain
    // float ch_gain = 14 * units::mV/units::fC, ch_shaping = 2.0 * units::us;

    double ch_gain = db_gain, ch_shaping = db_shaping;
    if (m_chanstat) {  // allow for deviation from nominal
        ch_gain = m_chanstat->preamp_gain(chid);
        ch_shaping = m_chanstat->preamp_shaping(chid);
    }
    double constant = lenamp->second.back();
    int nbin = lenamp->second.size() - 1;

    //    amplitude_t comb_amp;
    comb_amp = lenamp->second;
    comb_amp.pop_back();

    if (fabs(ch_gain - db_gain) > 0.01 * ch_gain) {
        // scale the amplitude, not the constant term ...
        for (int i = 0; i != nbin; i++) {
            comb_amp.at(i) *= ch_gain / db_gain;
        }
    }

    if (fabs(ch_shaping - db_shaping) > 0.01 * ch_shaping) {
        // scale the amplitude by different shaping time ...
        int nconfig = ch_shaping / units::us / 0.1;
        auto resp1 = m_elec_resp_cache.find(nconfig);
        if (resp1 == m_elec_resp_cache.end()) {
            Response::ColdElec elec_resp(10, ch_shaping);  // default at 1 mV/fC
            auto sig = elec_resp.generate(WireCell::Waveform::Domain(0, m_fft_length * m_period), m_fft_length);
            auto filt = Waveform::dft(sig);
            auto ele_resp_amp = Waveform::magnitude(filt);

            ele_resp_amp.resize(m_elec_resp_freq.size());
            m_elec_resp_cache[nconfig] = ele_resp_amp;
        }
        resp1 = m_elec_resp_cache.find(nconfig);

        nconfig = db_shaping / units::us / 0.1;
        auto resp2 = m_elec_resp_cache.find(nconfig);
        if (resp2 == m_elec_resp_cache.end()) {
            Response::ColdElec elec_resp(10, db_shaping);  // default at 1 mV/fC
            auto sig = elec_resp.generate(WireCell::Waveform::Domain(0, m_fft_length * m_period), m_fft_length);
            auto filt = Waveform::dft(sig);
            auto ele_resp_amp = Waveform::magnitude(filt);

            ele_resp_amp.resize(m_elec_resp_freq.size());
            m_elec_resp_cache[nconfig] = ele_resp_amp;
        }
        resp2 = m_elec_resp_cache.find(nconfig);

        for (int i = 0; i != nbin; i++) {
            comb_amp.at(i) *= resp1->second.at(i) / resp2->second.at(i);
        }
    }

    // add the constant terms ...
    for (int i = 0; i != nbin; i++) {
        comb_amp.at(i) = sqrt(pow(comb_amp.at(i), 2) + pow(constant, 2));  // units still in mV
    }

    return comb_amp;
}
