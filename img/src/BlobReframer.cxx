/*
  This starts by finding the set of all frames accessible via slices
  in a cluster that have at least one blob.



 */

#include "WireCellImg/BlobReframer.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

WIRECELL_FACTORY(BlobReframer, WireCell::Img::BlobReframer,
                 WireCell::INamed,
                 WireCell::IClusterFramer, WireCell::IConfigurable)

using namespace WireCell;

Img::BlobReframer::BlobReframer(const std::string& frame_tag)
    : Aux::Logger("BlobReframer", "img")
    , m_nticks(0)
    , m_period(0)
    , m_frame_tag(frame_tag)
{
}

Img::BlobReframer::~BlobReframer() {}

void Img::BlobReframer::configure(const WireCell::Configuration& cfg)
{
    // m_nticks = get(cfg, "nticks", m_nticks);
    m_frame_tag = get(cfg, "frame_tag", m_frame_tag);
}

WireCell::Configuration Img::BlobReframer::default_configuration() const
{
    Configuration cfg;
    // cfg["nticks"] = m_nticks;
    cfg["frame_tag"] = m_frame_tag;
    return cfg;
}

// cluster goes in, frame goes out.
bool Img::BlobReframer::operator()(const input_pointer& in, output_pointer& out)
{
    out = nullptr;
    if (!in) {
        return true;  // EOS
    }

    cluster_indexed_graph_t grind(in->graph());

    WireCell::IFrame::pointer iframe;
    std::unordered_map<int, ITrace::ChargeSequence> map_charges;
    for (auto iblob : oftype<IBlob::pointer>(grind)) {
        auto islice = iblob->slice();
        if (!iframe) {
            // typical values for protoDUNE
            // period: 0.5us, nticks: 6000
            iframe = islice->frame();
            m_period = iframe->tick();
            {
                std::vector<int> tbins;
                for (auto trace : *iframe->traces()) {
                    const int tbin = trace->tbin();
                    const int nbins = trace->charge().size();
                    tbins.push_back(tbin);
                    tbins.push_back(tbin + nbins);
                }
                auto mme = std::minmax_element(tbins.begin(), tbins.end());
                int tbinmin = *mme.first;
                int tbinmax = *mme.second;
                m_nticks = tbinmax - tbinmin;
                log->debug("nticks={} tbinmin={} tbinmax={}", m_nticks, tbinmin, tbinmax);
            }
        }
        int itick = islice->start() / m_period;
        int nspan = islice->span() / m_period;

        for (auto const& act : islice->activity()) {
            auto ichan = act.first;
            if (map_charges.find(ichan->ident()) == map_charges.end()) {
                map_charges.insert(std::make_pair(ichan->ident(), ITrace::ChargeSequence(m_nticks, 0.0)));
            }
            float q = act.second;
            for (int ns = 0; ns < nspan; ns++) {
                map_charges.at(ichan->ident()).at(itick + ns) += q / (float) nspan;
            }
        }
    }

    ITrace::vector* itraces = new ITrace::vector;  // will become shared_ptr.
    for (auto const& it : map_charges) {
        int ident = it.first;
        auto charge = it.second;
        // Save the waveform densely, including zeros.
        SimpleTrace* trace = new SimpleTrace(ident, 0, charge);
        itraces->push_back(ITrace::pointer(trace));
    }

    SimpleFrame* sframe =
        new SimpleFrame(iframe->ident(), iframe->time(), ITrace::shared_vector(itraces), iframe->tick());
    sframe->tag_frame(m_frame_tag);
    out = IFrame::pointer(sframe);

    return true;
}
