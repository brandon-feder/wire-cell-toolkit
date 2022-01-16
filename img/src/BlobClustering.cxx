#include "WireCellImg/BlobClustering.h"
#include "WireCellUtil/RayClustering.h"
#include "WireCellIface/SimpleCluster.h"
#include "WireCellUtil/NamedFactory.h"

#include <boost/graph/graphviz.hpp>

WIRECELL_FACTORY(BlobClustering, WireCell::Img::BlobClustering,
                 WireCell::INamed,
                 WireCell::IClustering, WireCell::IConfigurable)

using namespace WireCell;

Img::BlobClustering::BlobClustering()
    : Aux::Logger("BlobClustering", "img")
    , m_spans(1.0)
    , m_last_bs(nullptr)
{
}
Img::BlobClustering::~BlobClustering() {}

void Img::BlobClustering::configure(const WireCell::Configuration& cfg) { m_spans = get(cfg, "spans", m_spans); }

WireCell::Configuration Img::BlobClustering::default_configuration() const
{
    Configuration cfg;
    // A number multiplied to the span of the current slice when
    // determining it a gap exists between the next slice.  Default is
    // 1.0.  Eg, if set to 2.0 then a single missing slice won't be
    // grounds for considering a gap in the cluster.  A number less
    // than 1.0 will cause each "cluster" to consist of only one blob.
    cfg["spans"] = m_spans;
    return cfg;
}

void Img::BlobClustering::flush(output_queue& clusters)
{
    if (0 == boost::num_vertices(m_grind.graph())) {
        return;
    }
    clusters.push_back(std::make_shared<SimpleCluster>(m_grind.graph()));
    m_grind.clear();
    m_last_bs = nullptr;
}

void Img::BlobClustering::intern(const input_pointer& newbs) { m_last_bs = newbs; }

bool Img::BlobClustering::judge_gap(const input_pointer& newbs)
{
    const double epsilon = 1 * units::ns;

    if (m_spans <= epsilon) {
        return false;  // never break on gap.
    }

    auto nslice = newbs->slice();
    auto oslice = m_last_bs->slice();

    const double dt = nslice->start() - oslice->start();
    return std::abs(dt - m_spans * oslice->span()) > epsilon;
}

void Img::BlobClustering::add_slice(const ISlice::pointer& islice)
{
    if (m_grind.has(islice)) {
        return;
    }

    for (const auto& ichv : islice->activity()) {
        const IChannel::pointer ich = ichv.first;
        if (m_grind.has(ich)) {
            continue;
        }
        for (const auto& iwire : ich->wires()) {
            m_grind.edge(ich, iwire);
        }
    }
}

void Img::BlobClustering::add_blobs(const input_pointer& newbs)
{
    for (const auto& iblob : newbs->blobs()) {
        auto islice = iblob->slice();
        add_slice(islice);
        m_grind.edge(islice, iblob);

        auto iface = iblob->face();
        auto wire_planes = iface->planes();

        const auto& shape = iblob->shape();
        for (const auto& strip : shape.strips()) {
            // FIXME: need a way to encode this convention!
            // For now, it requires collusion.  Don't impeach me.
            const int num_nonplane_layers = 2;
            int iplane = strip.layer - num_nonplane_layers;
            if (iplane < 0) {
                continue;
            }
            const auto& wires = wire_planes[iplane]->wires();
            for (int wip = strip.bounds.first; wip < strip.bounds.second and wip < int(wires.size()); ++wip) {
                auto iwire = wires[wip];
                m_grind.edge(iblob, iwire);
            }
        }
    }
}

bool Img::BlobClustering::graph_bs(const input_pointer& newbs)
{
    add_blobs(newbs);

    if (!m_last_bs) {
        // need to wait for next one to do anything.
        // note, caller interns.
        return false;
    }
    if (judge_gap(newbs)) {
        // nothing to do, but pass on that we hit a gap
        return true;
    }

    // handle each face separately faces
    IBlob::vector iblobs1 = newbs->blobs();
    IBlob::vector iblobs2 = m_last_bs->blobs();

    RayGrid::blobs_t blobs1 = newbs->shapes();
    RayGrid::blobs_t blobs2 = m_last_bs->shapes();

    const auto beg1 = blobs1.begin();
    const auto beg2 = blobs2.begin();

    auto assoc = [&](RayGrid::blobref_t& a, RayGrid::blobref_t& b) {
        int an = a - beg1;
        int bn = b - beg2;
        m_grind.edge(iblobs1[an], iblobs2[bn]);
    };
    RayGrid::associate(blobs1, blobs2, assoc);

    return false;
}

bool Img::BlobClustering::operator()(const input_pointer& blobset, output_queue& clusters)
{
    if (!blobset) {  // eos
        flush(clusters);
        log->debug("flush {} clusters + EOS on EOS",
                 clusters.size());
        clusters.push_back(nullptr);  // forward eos
        return true;
    }

    bool gap = graph_bs(blobset);
    if (gap) {
        flush(clusters);
        log->debug("sending {} clusters after gap",
                   clusters.size());
        // note: flush fast to keep memory usage in this component
        // down and because in an MT job, downstream components might
        // benefit to start consuming clusters ASAP.  We do NOT want
        // to intern() the new blob set BEFORE a flush if there is a
        // gap because newbs is needed for next time and fush clears
        // the cache.
    }

    intern(blobset);

    SPDLOG_LOGGER_TRACE(log, "got {} blobs, holding graph with {}",
                        blobset->blobs().size(),
                        boost::num_vertices(m_grind.graph()));

    return true;
}
