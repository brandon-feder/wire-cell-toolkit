#include "WireCellGen/FrameSummer.h"

#include "WireCellAux/FrameTools.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleFrame.h"

WIRECELL_FACTORY(FrameSummer, WireCell::Gen::FrameSummer, WireCell::IFrameJoiner, WireCell::IConfigurable)

using namespace WireCell;

Configuration Gen::FrameSummer::default_configuration() const
{
    // fixme: maybe add operators, scaleing, offsets.

    Configuration cfg;

    // if true, the time of the second frame is ignored in favor of
    // the first.  Does not affect "tbin" values of individual traces.
    cfg["align"] = m_align;

    // Amount of time offset to apply to the time of the second frame.
    // If frame two is "aligned" then this offset is applied to frame
    // two relative to the time of frame one
    cfg["offset"] = m_toffset;

    return cfg;
}

void Gen::FrameSummer::configure(const Configuration& cfg)
{
    m_align = get(cfg, "align", m_align);
    m_toffset = get(cfg, "offset", m_toffset);
}

bool Gen::FrameSummer::operator()(const input_tuple_type& intup, output_pointer& out)
{
    auto one = std::get<0>(intup);
    auto two = std::get<1>(intup);
    if (!one or !two) {
        // assume eos
        out = nullptr;
        return true;
    }

    double t2 = two->time();
    if (m_align) {
        t2 = one->time();
    }
    t2 += m_toffset;

    auto vtraces2 = two->traces();
    ITrace::vector out_traces(vtraces2->begin(), vtraces2->end());
    auto newtwo = std::make_shared<SimpleFrame>(two->ident(), t2, out_traces, two->tick());

    out = Aux::sum(IFrame::vector{one, two}, one->ident());
    return true;
}

Gen::FrameSummer::FrameSummer()
  : m_toffset(0.0)
  , m_align(false)

{
}

Gen::FrameSummer::~FrameSummer() {}
