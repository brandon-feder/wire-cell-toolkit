#include "WireCellGen/Misconfigure.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

WIRECELL_FACTORY(Misconfigure, WireCell::Gen::Misconfigure, WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace WireCell;

Gen::Misconfigure::Misconfigure() {}

Gen::Misconfigure::~Misconfigure() {}

WireCell::Configuration Gen::Misconfigure::default_configuration() const
{
    Configuration cfg;

    // nominally correctly configured amplifiers in MB.  They should
    // match what ever was used to create the input waveforms.
    cfg["from"]["gain"] = 14.0 * units::mV / units::fC;
    cfg["from"]["shaping"] = 2.2 * units::us;

    // Nominally misconfigured amplifiers in MB.  They should match
    // whatever you wished the input waveforms would have been created
    // with.
    cfg["to"]["gain"] = 4.7 * units::mV / units::fC;
    cfg["to"]["shaping"] = 1.1 * units::us;

    /// The number of samples of the response functions.
    cfg["nsamples"] = 50;
    /// The period of sampling the response functions
    cfg["tick"] = 0.5 * units::us;

    /// If to truncate the misconfigured waveforms.  The convolution
    /// used to apply the misconfiguring will extend the a trace's
    /// waveform by nsamples-1.  Truncating will clip that much off so
    /// the waveform will remains the same length but some information
    /// may be lost.  If not truncated, this longer waveform likely
    /// needs to be handled in some way by the user.
    cfg["truncate"] = true;

    return cfg;
}

void Gen::Misconfigure::configure(const WireCell::Configuration& cfg)
{
    int n = cfg["nsamples"].asInt();
    double tick = cfg["tick"].asDouble();
    Binning bins(n, 0, n * tick);

    m_from = Response::ColdElec(cfg["from"]["gain"].asDouble(), cfg["from"]["shaping"].asDouble()).generate(bins);
    m_to = Response::ColdElec(cfg["to"]["gain"].asDouble(), cfg["to"]["shaping"].asDouble()).generate(bins);

    m_truncate = cfg["truncate"].asBool();
}

bool Gen::Misconfigure::operator()(const input_pointer& in, output_pointer& out)
{
    out = nullptr;
    if (!in) {
        return true;  // eos, but we don't care here.
    }

    auto traces = in->traces();
    if (!traces) {
        std::cerr << "Gen::Misconfigure: warning no traces in frame for me\n";
        return true;
    }

    size_t ntraces = traces->size();
    ITrace::vector out_traces(ntraces);
    for (size_t ind = 0; ind < ntraces; ++ind) {
        auto trace = traces->at(ind);

        auto wave = Waveform::replace_convolve(trace->charge(), m_to, m_from, m_truncate);
        out_traces[ind] = std::make_shared<SimpleTrace>(trace->channel(), trace->tbin(), wave);
    }

    out = std::make_shared<SimpleFrame>(in->ident(), in->time(), out_traces, in->tick());
    return true;
}
