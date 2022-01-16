#include "WireCellSigProc/FrameMerger.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/ITrace.h"

#include "WireCellAux/FrameTools.h"

#include <string>

WIRECELL_FACTORY(FrameMerger, WireCell::SigProc::FrameMerger, WireCell::IFrameJoiner, WireCell::IConfigurable)

using namespace WireCell;

Configuration SigProc::FrameMerger::default_configuration() const
{
    Configuration cfg;

    // The merge map specifies a list of triples of tags.  The first
    // tag in the pair corresponds to a trace tag on the first frame,
    // and likewise the second.  The third is a tag that will be
    // placed on the merged set of traces in the output frame.  The
    // merge is performed considering only traces in the two input
    // frames that respectively match their tags.  As tags may have
    // overlapping sets of traces, it is important to recognize that
    // the merge is progressive and in order of the merge map.  If the
    // merge map is empty then all traces in frame 1 are merged with
    // all traces in frame 2 irrespective of any tags.
    cfg["mergemap"] = Json::arrayValue;

    // The rule determins the algorithm employed in the merge.
    //
    // - replace :: the last trace encountered for a given channel is output.
    //   The traces from frame 1 will take precedence over any from frame 2.
    //
    // - include :: all traces encountered with a given channel are output.
    //
    // - tbd :: more may be added (eg, sum all traces on a given channel)
    cfg["rule"] = "replace";

    return cfg;
}

void SigProc::FrameMerger::configure(const Configuration& cfg) { m_cfg = cfg; }

bool SigProc::FrameMerger::operator()(const input_tuple_type& intup, output_pointer& out)
{
    out = nullptr;

    auto one = std::get<0>(intup);
    auto two = std::get<1>(intup);
    if (!one or !two) {
        std::cerr << "FrameMerger: EOS\n";
        return true;
    }

    auto jmergemap = m_cfg["mergemap"];
    const int nsets = jmergemap.size();

    // collect traces into a vector of vector whether we are dealling
    // with all traces or honoring tags.
    std::vector<ITrace::vector> tracesv1, tracesv2;
    if (!nsets) {
        tracesv1.push_back(Aux::untagged_traces(one));
        tracesv2.push_back(Aux::untagged_traces(two));
        std::cerr << "FrameMerger: see frame: " << one->ident() << " no tags, whole frame\n";
    }
    else {
        std::cerr << "FrameMerger: see frame: " << one->ident() << " with tags:\n";
        for (int ind = 0; ind < nsets; ++ind) {
            auto jtags = jmergemap[ind];
            std::string tag1 = jtags[0].asString();
            std::string tag2 = jtags[1].asString();
            std::string tag3 = jtags[2].asString();
            tracesv1.push_back(Aux::tagged_traces(one, tag1));
            tracesv2.push_back(Aux::tagged_traces(two, tag2));
            std::cerr << "\ttags: " << tag1 << "[" << tracesv1.back().size() << "]"
                      << " + " << tag2 << "[" << tracesv2.back().size() << "]"
                      << " -> " << tag3 << "\n";
        }
    }

    ITrace::vector out_traces;
    std::vector<IFrame::trace_list_t> tagged_trace_indices;

    // apply rule, collect info for tags even if we may not be
    // configured to honor them.

    auto rule = get<std::string>(m_cfg, "rule");
    if (rule == "replace") {
        for (size_t ind = 0; ind < tracesv1.size(); ++ind) {
            auto& traces1 = tracesv1[ind];
            auto& traces2 = tracesv2[ind];

            std::unordered_map<int, ITrace::pointer> ch2tr;
            for (auto trace : traces2) {
                ch2tr[trace->channel()] = trace;
            }
            for (auto trace : traces1) {  // now replace any from frame 1
                ch2tr[trace->channel()] = trace;
            }

            IFrame::trace_list_t tl;
            for (auto chtr : ch2tr) {
                tl.push_back(out_traces.size());
                out_traces.push_back(chtr.second);
            }
            tagged_trace_indices.push_back(tl);
        }
    }
    if (rule == "include") {
        for (size_t ind = 0; ind < tracesv1.size(); ++ind) {
            auto& traces1 = tracesv1[ind];
            auto& traces2 = tracesv2[ind];

            IFrame::trace_list_t tl;
            for (size_t trind = 0; trind < traces1.size(); ++trind) {
                tl.push_back(out_traces.size());
                out_traces.push_back(traces1[trind]);
            }
            for (size_t trind = 0; trind < traces2.size(); ++trind) {
                tl.push_back(out_traces.size());
                out_traces.push_back(traces2[trind]);
            }
            tagged_trace_indices.push_back(tl);
        }
    }

    auto sf = new SimpleFrame(two->ident(), two->time(), out_traces, two->tick());
    if (nsets) {
        for (int ind = 0; ind < nsets; ++ind) {
            std::string otag = jmergemap[ind][2].asString();
            sf->tag_traces(otag, tagged_trace_indices[ind]);
        }
    }
    out = IFrame::pointer(sf);
    return true;
}

SigProc::FrameMerger::FrameMerger() {}

SigProc::FrameMerger::~FrameMerger() {}
