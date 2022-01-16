#include "WireCellGen/SilentNoise.h"  // holly noise
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Units.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include <memory>

WIRECELL_FACTORY(SilentNoise, WireCell::Gen::SilentNoise, WireCell::IFrameSource, WireCell::IConfigurable)

using namespace WireCell;

Gen::SilentNoise::SilentNoise()
  : m_count(0)
{
}

Gen::SilentNoise::~SilentNoise() {}

void Gen::SilentNoise::configure(const WireCell::Configuration& cfg)
{
    m_noutputs = get(cfg, "noutputs", 0);
    m_nchannels = get(cfg, "nchannels", 0);
    m_traces_tag = cfg["traces_tag"].asString();
}

WireCell::Configuration Gen::SilentNoise::default_configuration() const
{
    Configuration cfg;
    cfg["noutputs"] = 0;     // run forever
    cfg["nchannels"] = 0;    // empty
    cfg["traces_tag"] = "";  // empty string means no tag on traces
    return cfg;
}

bool Gen::SilentNoise::operator()(output_pointer& out)
{
    out = nullptr;
    if (m_noutputs and m_count == m_noutputs) {
        ++m_count;
        return true;
    }
    if (m_noutputs and m_count >= m_noutputs) {
        return false;
    }
    // std::cerr << "SilentNoise: output #" << m_count << " / " << m_noutputs << std::endl;
    ITrace::vector traces(m_nchannels);
    for (int ind = 0; ind < m_nchannels; ++ind) {
        traces[ind] = std::make_shared<SimpleTrace>(ind, 0, 0);  // boring traces
    }
    auto sfout = new SimpleFrame(m_count, m_count * 5.0 * units::ms, traces);
    if (m_traces_tag != "") {
        IFrame::trace_list_t tl(m_nchannels);
        std::iota(tl.begin(), tl.end(), 0);
        sfout->tag_traces(m_traces_tag, tl);
    }
    out = IFrame::pointer(sfout);
    ++m_count;
    return true;
}
