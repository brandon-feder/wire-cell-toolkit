#include "WireCellGen/PerChannelVariation.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include <string>

WIRECELL_FACTORY(PerChannelVariation, WireCell::Gen::PerChannelVariation, WireCell::IFrameFilter,
                 WireCell::IConfigurable)

using namespace WireCell;

Gen::PerChannelVariation::PerChannelVariation() {}

Gen::PerChannelVariation::~PerChannelVariation() {}

WireCell::Configuration Gen::PerChannelVariation::default_configuration() const
{
    Configuration cfg;

    // nominally correctly configured amplifiers in MB.  They should
    // match what ever was used to create the input waveforms.
    cfg["gain"] = 14.0 * units::mV / units::fC;
    cfg["shaping"] = 2.2 * units::us;

    // The number of nsamples of the response functions.
    cfg["nsamples"] = 310;
    // The period of sampling the response functions
    cfg["tick"] = 0.5 * units::us;

    /// If to truncate the waveforms.  The convolution
    /// used to apply the misconfiguring will extend the a trace's
    /// waveform by nsamples-1.  Truncating will clip that much off so
    /// the waveform will remains the same length but some information
    /// may be lost.  If not truncated, this longer waveform likely
    /// needs to be handled in some way by the user.
    cfg["truncate"] = true;

    /// ch-by-ch electronics responses by calibration
    cfg["per_chan_resp"] = "";

    return cfg;
}

void Gen::PerChannelVariation::configure(const WireCell::Configuration& cfg)
{
    m_per_chan_resp = get<std::string>(cfg, "per_chan_resp", "");

    if (!m_per_chan_resp.empty()) {
        std::cerr << "SIMULATION: CH-BY-CH ELECTRONICS RESPONSE VARIATION\n";
        m_cr = Factory::find_tn<IChannelResponse>(m_per_chan_resp);
        double tick = cfg["tick"].asDouble();
        auto cr_bins = m_cr->channel_response_binning();
        if (cr_bins.binsize() != tick) {
            THROW(ValueError() << errmsg{"PerChannelVariation: channel response tbin size mismatch!"});
        }
        m_nsamples = cr_bins.nbins();
        WireCell::Binning tbins(m_nsamples, cr_bins.min(), cr_bins.min() + m_nsamples * tick);
        m_from = Response::ColdElec(cfg["gain"].asDouble(), cfg["shaping"].asDouble()).generate(tbins);
    }

    m_truncate = cfg["truncate"].asBool();
}

bool Gen::PerChannelVariation::operator()(const input_pointer& in, output_pointer& out)
{
    if (!in) {
        out = nullptr;
        return true;
    }

    if (m_per_chan_resp.empty()) {
        std::cerr << "Gen::PerChannelVariation: warning no ch-by-ch response was found!\n";
        out = in;
        return true;
    }

    auto traces = in->traces();
    if (!traces) {
        std::cerr << "Gen::PerChannelVariation: error no traces in frame for me\n";
        return false;
    }

    size_t ntraces = traces->size();
    ITrace::vector out_traces(ntraces);
    for (size_t ind = 0; ind < ntraces; ++ind) {
        auto trace = traces->at(ind);
        auto chid = trace->channel();
        Waveform::realseq_t tch_resp = m_cr->channel_response(chid);
        tch_resp.resize(m_nsamples, 0);
        auto wave = Waveform::replace_convolve(trace->charge(), tch_resp, m_from, m_truncate);
        out_traces[ind] = std::make_shared<SimpleTrace>(chid, trace->tbin(), wave);
    }

    out = std::make_shared<SimpleFrame>(in->ident(), in->time(), out_traces, in->tick());
    return true;
}
