#include "WireCellAux/PlaneSelector.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellAux/FrameTools.h"
#include "WireCellAux/PlaneTools.h"

#include "WireCellUtil/NamedFactory.h"

#include <sstream>

WIRECELL_FACTORY(PlaneSelector, WireCell::Aux::PlaneSelector,
                 WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace WireCell;

Aux::PlaneSelector::PlaneSelector()
    : Aux::Logger("PlaneSelector", "glue")
{
}

Aux::PlaneSelector::~PlaneSelector() {}

WireCell::Configuration Aux::PlaneSelector::default_configuration() const
{
    Configuration cfg;

    /// The anode to match
    cfg["anode"] = "AnodePlane";

    /// The plane number.  Currently this is interpreted as the plane
    /// index in [0,1,2].  For 2-faced anodes, this will select
    /// channels with this plane index from both faces in face-minor
    /// order (first face 0 channels then face 1 channels).
    cfg["plane"] = 0;

    // A future extension may add a "method" config parameter which is
    // used to interpret "plane" number against "ident", "ilayer" or
    // "index" (which "index" being the default and current
    // interpretation).

    /// Input trace tags to consider for selection.  If this list is
    /// empty, all traces are considered for selection and all input
    /// tags will be transfered via tag rules.  If tags are given then
    /// only input traces of these tags are considered.
    cfg["tags"] = Json::arrayValue;

    /// Rules to govern the output frame and trace tags based on input
    /// frame and trace tags.  There is no map from input frame or
    /// trace tags then it will be kept for the output frame or trace
    /// tag.
    cfg["tag_rules"] = Json::arrayValue;

    return cfg;
}

void Aux::PlaneSelector::configure(const WireCell::Configuration& cfg)
{
    int wire_plane_index = get(cfg, "plane", 0);
    
    // We only need the anode temporarily to build set of channel IDs
    // that we care about.
    std::string anode_tn = get<std::string>(cfg, "anode", "AnodePlane");
    auto anode = Factory::find_tn<IAnodePlane>(anode_tn);
    auto chans = Aux::plane_channels(anode, wire_plane_index);
    m_chids.clear();
    for (const auto& ichan : chans) {
        m_chids.insert(ichan->ident());
    }
    {
        auto chmm = std::minmax_element(m_chids.begin(), m_chids.end());
        log->debug("anode={} plane={} nchans={} in=[{},{}]",
                   anode_tn, wire_plane_index, m_chids.size(),
                   *chmm.first, *chmm.second);
    }
    
    auto jtags = cfg["tags"];
    m_tags.clear();
    for (auto jtag : cfg["tags"]) {
        m_tags.push_back(jtag.asString());
    }

    auto tr = cfg["tag_rules"];
    m_ft.configure(tr);
}


bool Aux::PlaneSelector::operator()(const input_pointer& in,
                                    output_pointer& out)
{
    out = nullptr;
    if (!in) {
        log->debug("see EOS at call={}", m_count);
        ++m_count;
        return true;  // eos
    }

    IFrame::tag_list_t my_tags = m_tags;
    if (my_tags.empty()) {
        my_tags = in->trace_tags();
        my_tags.push_back("");  // marker for "untagged traces"
    }

    ITrace::vector out_traces;
    std::map<std::string, IFrame::trace_list_t> tagged_indices;

    for (const auto& my_tag : my_tags) {
        ITrace::vector traces;
        if (my_tag.empty()) {
            traces = Aux::untagged_traces(in);
        }
        else {
            traces = Aux::tagged_traces(in, my_tag);
        }

        IFrame::trace_list_t indices;
        for (auto itrace : traces) {
            int chid = itrace->channel();
            if (m_chids.find(chid) == m_chids.end()) {
                continue;
            }
            indices.push_back(out_traces.size());
            out_traces.push_back(itrace);
        }

        auto new_tags = m_ft.transform(0, "trace", {my_tag});
        if (new_tags.empty()) {
            log->debug("call={} no transform, keeping input tag: {}", m_count, my_tag);
            new_tags.insert(my_tag);
        }
        for (auto new_tag : new_tags) {
            tagged_indices[new_tag] = indices;
        }
    }

    auto sf = new SimpleFrame(in->ident(), in->time(), out_traces, in->tick());
    for (auto& [new_tag,indices] : tagged_indices) {
        sf->tag_traces(new_tag, indices);
    }

    std::vector<std::string> frame_tags = in->frame_tags();
    if (frame_tags.empty()) {
        frame_tags.push_back("");
    }
    auto new_tags = m_ft.transform(0, "frame", frame_tags);
    for (auto new_tag : new_tags) {
        if (new_tag.empty()) {
            continue;
        }
        sf->tag_frame(new_tag);
    }
    out = IFrame::pointer(sf);

    log->debug("call={} input {}", m_count, Aux::taginfo(in));
    log->debug("call={} output {}", m_count, Aux::taginfo(out));

    ++m_count;
    return true;
}

