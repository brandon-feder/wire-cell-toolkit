#include "WireCellIface/SimpleFrame.h"

using namespace WireCell;
using namespace std;

SimpleFrame::SimpleFrame(int ident, double time, const ITrace::vector& traces, double tick,
                         const Waveform::ChannelMaskMap& cmm)
  : m_ident(ident)
  , m_time(time)
  , m_tick(tick)
  , m_traces(new ITrace::vector(traces.begin(), traces.end()))
  , m_cmm(cmm)
{
}
SimpleFrame::SimpleFrame(int ident, double time, ITrace::shared_vector traces, double tick,
                         const Waveform::ChannelMaskMap& cmm)
  : m_ident(ident)
  , m_time(time)
  , m_tick(tick)
  , m_traces(traces)
  , m_cmm(cmm)
{
}
SimpleFrame::~SimpleFrame() {}
int SimpleFrame::ident() const { return m_ident; }
double SimpleFrame::time() const { return m_time; }
double SimpleFrame::tick() const { return m_tick; }

ITrace::shared_vector SimpleFrame::traces() const { return m_traces; }

Waveform::ChannelMaskMap SimpleFrame::masks() const { return m_cmm; }

SimpleFrame::SimpleTraceInfo::SimpleTraceInfo()
  : indices(0)
  , summary(0)
{
}
const SimpleFrame::SimpleTraceInfo& SimpleFrame::get_trace_info(const IFrame::tag_t& tag) const
{
    static SimpleTraceInfo empty;
    auto const& it = m_trace_info.find(tag);
    if (it == m_trace_info.end()) {
        return empty;
    }
    return it->second;
}

const IFrame::tag_list_t& SimpleFrame::frame_tags() const { return m_frame_tags; }
const IFrame::tag_list_t& SimpleFrame::trace_tags() const { return m_trace_tags; }

const IFrame::trace_list_t& SimpleFrame::tagged_traces(const tag_t& tag) const { return get_trace_info(tag).indices; }

const IFrame::trace_summary_t& SimpleFrame::trace_summary(const tag_t& tag) const
{
    return get_trace_info(tag).summary;
}

void SimpleFrame::tag_frame(const tag_t& tag) { m_frame_tags.push_back(tag); }

void SimpleFrame::tag_traces(const tag_t& tag, const IFrame::trace_list_t& indices,
                             const IFrame::trace_summary_t& summary)
{
    auto& info = m_trace_info[tag];
    info.indices = indices;
    info.summary = summary;

    // Kind of dumb way to update this but we want to be able to
    // return a reference to it later.
    m_trace_tags.clear();
    for (auto& it : m_trace_info) {
        m_trace_tags.push_back(it.first);
    }
}
