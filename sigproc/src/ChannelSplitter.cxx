#include "WireCellSigProc/ChannelSplitter.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/IAnodePlane.h"

WIRECELL_FACTORY(ChannelSplitter, WireCell::SigProc::ChannelSplitter, WireCell::IFrameFanout, WireCell::IConfigurable)

using namespace WireCell;

SigProc::ChannelSplitter::ChannelSplitter(size_t multiplicity)
  : m_multiplicity(multiplicity)
  , log(Log::logger("glue"))
{
}
SigProc::ChannelSplitter::~ChannelSplitter() {}

WireCell::Configuration SigProc::ChannelSplitter::default_configuration() const
{
    Configuration cfg;

    // A list of anode names
    cfg["anodes"] = Json::arrayValue;
    // Tag rules are an array, one element per output port.  Each
    // element is an object keyed with "frame" or "trace".  Each of
    // their values are an object keyed by a regular expression
    // (regex) and with values that are a single tag or an array of
    // tags.  See util/test_tagrules for examples.
    cfg["tag_rules"] = Json::arrayValue;

    return cfg;
}
void SigProc::ChannelSplitter::configure(const WireCell::Configuration& cfg)
{
    m_multiplicity = 0;
    m_c2p.clear();
    for (auto janode : cfg["anodes"]) {
        auto anode = Factory::find_tn<IAnodePlane>(janode.asString());
        for (auto c : anode->channels()) {
            m_c2p[c] = m_multiplicity;
        }
        ++m_multiplicity;
    }
    if (m_c2p.empty()) {
        THROW(ValueError() << errmsg{"ChannelSplitter must have at least one annode"});
    }
    m_ft.configure(cfg["tag_rules"]);
}

std::vector<std::string> SigProc::ChannelSplitter::output_types()
{
    const std::string tname = std::string(typeid(output_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool SigProc::ChannelSplitter::operator()(const input_pointer& in, output_vector& outv)
{
    outv.resize(m_multiplicity);

    if (!in) {  //  pass on EOS
        for (size_t ind = 0; ind < m_multiplicity; ++ind) {
            outv[ind] = in;
        }
        log->debug("ChannelSplitter: see EOS");
        return true;
    }

    std::vector<ITrace::vector> port_traces(m_multiplicity);

    for (const auto& itrace : *(in->traces())) {
        int chan = itrace->channel();
        const auto& it = m_c2p.find(chan);
        if (it == m_c2p.end()) {
            log->debug("ChannelSplitter: no port for channel {}, dropping", chan);
            continue;
        }
        const int port = it->second;
        port_traces[port].push_back(itrace);
    }

    auto fintags = in->frame_tags();
    std::stringstream taginfo;
    for (size_t iport = 0; iport < m_multiplicity; ++iport) {
        // Basic frame stays the same.
        auto sfout = new SimpleFrame(in->ident(), in->time(), port_traces[iport], in->tick());

        // Transform any frame tags based on a per output port ruleset
        auto fouttags = m_ft.transform(iport, "frame", fintags);

        for (auto ftag : fouttags) {
            sfout->tag_frame(ftag);
            taginfo << " ftag:" << ftag;
        }

        for (auto inttag : in->trace_tags()) {
            tagrules::tagset_t touttags = m_ft.transform(iport, "trace", inttag);
            if (touttags.empty()) {
                continue;
            }
            const auto& traces = in->tagged_traces(inttag);
            const auto& summary = in->trace_summary(inttag);
            for (auto otag : touttags) {
                sfout->tag_traces(otag, traces, summary);
                taginfo << " " << inttag << "->" << otag;
            }
        };

        outv[iport] = IFrame::pointer(sfout);
    }

    std::string tagmsg = taginfo.str();
    if (!tagmsg.empty()) {
        log->debug("ChannelSplitter: tagnifo:{}", taginfo.str());
    }
    return true;
}
