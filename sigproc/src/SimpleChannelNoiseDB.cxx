#include "WireCellSigProc/SimpleChannelNoiseDB.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Binning.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(testChannelNoiseDB, WireCell::SigProc::SimpleChannelNoiseDB, WireCell::IChannelNoiseDatabase)

using namespace WireCell;
using namespace WireCell::SigProc;

SimpleChannelNoiseDB::SimpleChannelNoiseDB(double tick, int nsamples)
  : m_tick(-1)
  , m_nsamples(-1)
  , m_default_baseline(0.0)
  , m_default_gain(1.0)
  , m_default_offset(0.0)
  , m_default_min_rms(0.5)
  , m_default_max_rms(10)
  , m_default_pad_f(0)
  , m_default_pad_b(0)
  , m_default_decon_limit(0.02)
  , m_default_decon_lf_cutoff(0.08)
  , m_default_adc_limit(15.0)
  , m_default_decon_limit1(0.08)
  , m_default_protection_factor(5.0)
  , m_default_min_adc_limit(50)
  , m_default_roi_min_max_ratio(0.8)
{
    set_sampling(tick, nsamples);
}
SimpleChannelNoiseDB::~SimpleChannelNoiseDB() {}

double SimpleChannelNoiseDB::nominal_baseline(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_baseline.size()) {
        return m_baseline[ind];
    }
    return m_default_baseline;
}

double SimpleChannelNoiseDB::gain_correction(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_gain.size()) {
        return m_gain[ind];
    }
    return m_default_gain;
}

double SimpleChannelNoiseDB::response_offset(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_offset.size()) {
        return m_offset[ind];
    }
    return m_default_offset;
}

double SimpleChannelNoiseDB::min_rms_cut(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_min_rms.size()) {
        return m_min_rms[ind];
    }
    return m_default_min_rms;
}

double SimpleChannelNoiseDB::max_rms_cut(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_max_rms.size()) {
        return m_max_rms[ind];
    }
    return m_default_max_rms;
}

int SimpleChannelNoiseDB::pad_window_front(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_pad_f.size()) {
        return m_pad_f[ind];
    }
    return m_default_pad_f;
}

int SimpleChannelNoiseDB::pad_window_back(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_pad_b.size()) {
        return m_pad_b[ind];
    }
    return m_default_pad_b;
}

float SimpleChannelNoiseDB::coherent_nf_decon_limit(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_decon_limit.size()) {
        return m_decon_limit[ind];
    }
    return m_default_decon_limit;
}

float SimpleChannelNoiseDB::coherent_nf_decon_lf_cutoff(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_decon_lf_cutoff.size()) {
        return m_decon_lf_cutoff[ind];
    }
    return m_default_decon_lf_cutoff;
}

float SimpleChannelNoiseDB::coherent_nf_decon_limit1(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_decon_limit1.size()) {
        return m_decon_limit1[ind];
    }
    return m_default_decon_limit1;
}

float SimpleChannelNoiseDB::coherent_nf_adc_limit(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_adc_limit.size()) {
        return m_adc_limit[ind];
    }
    return m_default_adc_limit;
}

float SimpleChannelNoiseDB::coherent_nf_protection_factor(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_protection_factor.size()) {
        return m_protection_factor[ind];
    }
    return m_default_protection_factor;
}

float SimpleChannelNoiseDB::coherent_nf_min_adc_limit(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_min_adc_limit.size()) {
        return m_min_adc_limit[ind];
    }
    return m_default_min_adc_limit;
}

float SimpleChannelNoiseDB::coherent_nf_roi_min_max_ratio(int channel) const
{
    const int ind = chind(channel);
    if (0 <= ind && ind < (int) m_roi_min_max_ratio.size()) {
        return m_roi_min_max_ratio[ind];
    }
    return m_default_roi_min_max_ratio;
}

const IChannelNoiseDatabase::filter_t& SimpleChannelNoiseDB::get_filter(int channel, const filter_vector_t& fv) const
{
    const int ind = chind(channel);
    // std::cerr << "ch=" << channel << " ind=" << ind << " " << fv.size() << std::endl;
    if (0 <= ind && ind < (int) fv.size()) {
        const shared_filter_t sf = fv[ind];
        if (sf == nullptr) {
            return *(m_default_filter.get());
        }

        const filter_t* filtp = sf.get();
        return *filtp;
    }
    const filter_t* filtp = m_default_filter.get();
    // std::cerr << "Filter: "<< (void*)filtp << std::endl;
    return *filtp;
}

const IChannelNoiseDatabase::filter_t& SimpleChannelNoiseDB::rcrc(int channel) const
{
    return get_filter(channel, m_rcrc);
}

const IChannelNoiseDatabase::filter_t& SimpleChannelNoiseDB::config(int channel) const
{
    return get_filter(channel, m_config);
}

const IChannelNoiseDatabase::filter_t& SimpleChannelNoiseDB::noise(int channel) const
{
    return get_filter(channel, m_masks);
}

const IChannelNoiseDatabase::filter_t& SimpleChannelNoiseDB::response(int channel) const
{
    return get_filter(channel, m_response);
}

void SimpleChannelNoiseDB::set_sampling(double tick, int nsamples)
{
    if (m_nsamples == nsamples && tick == m_tick) {
        // std::cerr << "Sampling unchanged: " << nsamples << " @ " << m_tick/units::ms << " ms" << std::endl;
        return;
    }
    m_nsamples = nsamples;
    m_tick = tick;

    m_rcrc.clear();
    m_config.clear();
    m_response.clear();

    Waveform::compseq_t spectrum;
    spectrum.resize(nsamples, std::complex<float>(1, 0));
    m_default_filter = std::make_shared<filter_t>(spectrum);
    Waveform::compseq_t empty;
    m_default_response = std::make_shared<filter_t>(empty);
}

// set one thing in a vector at the index, resizing if needed, use def
// to back fill in case of resizing
template <typename T>
void set_one(int ind, T val, std::vector<T>& vec, T def)
{
    if (ind >= (int) vec.size()) {
        vec.resize(ind + 1, def);
    }
    vec[ind] = val;
}

void SimpleChannelNoiseDB::set_nominal_baseline(const std::vector<int>& channels, double baseline)
{
    // std::cerr << "SimpleChannelNoiseDB: set baseline to " << channels.size() << " chans: " << baseline << std::endl;
    for (auto ch : channels) {
        set_one(chind(ch), baseline, m_baseline, m_default_baseline);
    }
}
void SimpleChannelNoiseDB::set_rcrc_constant(const std::vector<int>& channels, double rcrc)
{
    Response::SimpleRC rcres(rcrc, m_tick);
    // auto signal = rcres.generate(WireCell::Waveform::Domain(0, m_nsamples*m_tick), m_nsamples);
    //    auto signal = rcres.generate(WireCell::Binning(m_nsamples, 0, m_nsamples*m_tick));
    auto signal = rcres.generate(WireCell::Waveform::Domain(0, m_nsamples * m_tick), m_nsamples);

    Waveform::compseq_t spectrum = Waveform::dft(signal);

    // std::cout << rcrc << " " << m_tick << " " << m_nsamples << " " << signal.front() << " " << signal.at(1) << " " <<
    // signal.at(2) << std::endl;

    // get the square of it because there are two RC filters
    Waveform::compseq_t spectrum2 = spectrum;
    Waveform::scale(spectrum2, spectrum);
    // for (auto it : spectrum){
    //   float real_part = it.real();
    //   float imag_part = it.imag();
    //   std::complex<float> temp(real_part*real_part-imag_part*imag_part,2*real_part*imag_part);
    //   spectrum2.push_back(temp);
    // }

    // std::cerr << "SimpleChannelNoiseDB:: get rcrc as: " << rcrc
    //           << " sum=" << Waveform::sum(spectrum2)
    //           << std::endl;

    auto filt = std::make_shared<filter_t>(spectrum2);

    for (auto ch : channels) {
        set_one(chind(ch), filt, m_rcrc, m_default_filter);
    }
}

void SimpleChannelNoiseDB::set_response(const std::vector<int>& channels, const filter_t& spectrum)
{
    // std::cerr << "SimpleChannelNoiseDB: set respnose on " << channels.size() << " chans with: " << spectrum.size() <<
    // " samples\n";

    auto filt = std::make_shared<filter_t>(spectrum);
    for (auto ch : channels) {
        set_one(chind(ch), filt, m_response, m_default_response);
    }
}

void SimpleChannelNoiseDB::set_gains_shapings(const std::vector<int>& channels, double from_gain, double to_gain,
                                              double from_shaping, double to_shaping)
{
    const double gain_ratio = to_gain / from_gain;
    // std::cerr << "SimpleChannelNoiseDB: set gain/shaping on " << channels.size() << " chans to: "
    //           << "g=" << from_gain << "->" << to_gain << ", "
    //           << "s=" << from_shaping << "->" << to_shaping << ", "
    //           << "rat=" << gain_ratio
    //           << "m_tick=" << m_tick/units::us << " us"
    //           << "\n";

    Response::ColdElec from_ce(from_gain, from_shaping);
    Response::ColdElec to_ce(to_gain, to_shaping);
    // auto to_sig   =   to_ce.generate(WireCell::Binning(m_nsamples, 0, m_nsamples*m_tick));
    // auto from_sig = from_ce.generate(WireCell::Binning(m_nsamples, 0, m_nsamples*m_tick));
    auto to_sig = to_ce.generate(WireCell::Waveform::Domain(0, m_nsamples * m_tick), m_nsamples);
    auto from_sig = from_ce.generate(WireCell::Waveform::Domain(0, m_nsamples * m_tick), m_nsamples);

    auto to_filt = Waveform::dft(to_sig);
    auto from_filt = Waveform::dft(from_sig);

    // auto from_filt_sum = Waveform::sum(from_filt);
    // auto to_filt_sum   = Waveform::sum(to_filt);

    Waveform::shrink(to_filt, from_filt);  // divide
    auto filt = std::make_shared<filter_t>(to_filt);

    // std::cout << filt->at(0) << " " << filt->at(1) << std::endl;
    // std::cerr << "SimpleChannelNoiseDB: "
    //           << " from_sig sum=" << Waveform::sum(from_sig)
    //           << " to_sig sum=" << Waveform::sum(to_sig)
    //           << " from_filt sum=" << from_filt_sum
    //           << " to_filt sum=" << to_filt_sum
    //           << " rat_filt sum=" << Waveform::sum(to_filt)
    //           << std::endl;

    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, filt, m_config, m_default_filter);
        set_one(ind, gain_ratio, m_gain, m_default_gain);
    }
}

void SimpleChannelNoiseDB::set_response_offset(const std::vector<int>& channels, double offset)
{
    // std::cerr << "SimpleChannelNoiseDB: set response offset " << channels.size() << " chans to: " << offset <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, offset, m_offset, m_default_offset);
    }
}

void SimpleChannelNoiseDB::set_min_rms_cut(const std::vector<int>& channels, double min_rms)
{
    // std::cerr << "SimpleChannelNoiseDB: set min rms cut on "<<channels.size()<<":[" << channels.front() << "," <<
    // channels.back() << "] to: " << min_rms << std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, min_rms, m_min_rms, m_default_min_rms);
    }
}

void SimpleChannelNoiseDB::set_min_rms_cut_one(int ch, double min_rms)
{
    // std::cerr << "SimpleChannelNoiseDB: set min rms cut on " << ch << " to: " << min_rms << std::endl;
    int ind = chind(ch);
    set_one(ind, min_rms, m_min_rms, m_default_min_rms);
}

void SimpleChannelNoiseDB::set_max_rms_cut(const std::vector<int>& channels, double max_rms)
{
    // std::cerr << "SimpleChannelNoiseDB: set max rms cut on "<<channels.size()<<":[" << channels.front() << "," <<
    // channels.back() << "] to: " << max_rms << std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, max_rms, m_max_rms, m_default_max_rms);
    }
}

void SimpleChannelNoiseDB::set_max_rms_cut_one(int ch, double max_rms)
{
    // std::cerr << "SimpleChannelNoiseDB: set max rms cut on " << ch << " to: " << max_rms << std::endl;
    int ind = chind(ch);
    set_one(ind, max_rms, m_max_rms, m_default_max_rms);
}

void SimpleChannelNoiseDB::set_pad_window_front(const std::vector<int>& channels, int pad_f)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window front on " << channels.size() << " channels: " << pad_f <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, pad_f, m_pad_f, m_default_pad_f);
    }
}

void SimpleChannelNoiseDB::set_pad_window_back(const std::vector<int>& channels, int pad_b)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, pad_b, m_pad_b, m_default_pad_b);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_decon_limit(const std::vector<int>& channels, float decon_limit)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, decon_limit, m_decon_limit, m_default_decon_limit);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_decon_lf_cutoff(const std::vector<int>& channels, float decon_lf_cutoff)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, decon_lf_cutoff, m_decon_lf_cutoff, m_default_decon_lf_cutoff);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_decon_limit1(const std::vector<int>& channels, float decon_limit1)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, decon_limit1, m_decon_limit1, m_default_decon_limit1);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_adc_limit(const std::vector<int>& channels, float adc_limit)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, adc_limit, m_adc_limit, m_default_adc_limit);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_protection_factor(const std::vector<int>& channels, float protection_factor)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, protection_factor, m_protection_factor, m_default_protection_factor);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_min_adc_limit(const std::vector<int>& channels, float min_adc_limit)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, min_adc_limit, m_min_adc_limit, m_default_min_adc_limit);
    }
}

void SimpleChannelNoiseDB::set_coherent_nf_roi_min_max_ratio(const std::vector<int>& channels, float roi_min_max_ratio)
{
    // std::cerr << "SimpleChannelNoiseDB: set pad window back on " << channels.size() << " channels: " << pad_b <<
    // std::endl;
    for (auto ch : channels) {
        int ind = chind(ch);
        set_one(ind, roi_min_max_ratio, m_roi_min_max_ratio, m_default_roi_min_max_ratio);
    }
}

void SimpleChannelNoiseDB::set_filter(const std::vector<int>& channels, const multimask_t& masks)
{
    Waveform::compseq_t spectrum;
    spectrum.resize(m_nsamples, std::complex<float>(1, 0));

    for (auto m : masks) {
        for (int ind = get<1>(m); ind <= get<2>(m); ++ind) {
            // filt->assign(ind, get<0>(m));
            spectrum.at(ind) = get<0>(m);
        }
        // std::cerr << "set freqmasks: " << get<1>(m) << " " << get<2>(m) << " " << get<0>(m) << " " << spectrum.at(0)
        // << " " << spectrum.at(169) << " " << spectrum.at(170)<< std::endl;
    }
    auto filt = std::make_shared<filter_t>(spectrum);

    for (auto ch : channels) {
        set_one(chind(ch), filt, m_masks, m_default_filter);
    }
}

void SimpleChannelNoiseDB::set_channel_groups(const std::vector<channel_group_t>& channel_groups)
{
    // std::cerr << "SimpleChannelNoiseDB: channel groups: " << channel_groups.size() << " X " <<
    // channel_groups[0].size() << std::endl;
    m_channel_groups = channel_groups;
}

void SimpleChannelNoiseDB::set_bad_channels(const channel_group_t& bc)
{
    // std::cerr << "SimpleChannelNoiseDB:: bad channels: " << bc.size() << std::endl;
    m_bad_channels = bc;
}

int SimpleChannelNoiseDB::chind(int ch) const
{
    auto it = m_ch2ind.find(ch);
    if (it == m_ch2ind.end()) {
        int ind = m_ch2ind.size();
        m_ch2ind[ch] = ind;
        return ind;
    }
    return it->second;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
