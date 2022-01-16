#include "WireCellImg/BlobSolving.h"
#include "WireCellIface/ICluster.h"
#include "WireCellIface/SimpleBlob.h"
#include "WireCellIface/SimpleCluster.h"
#include "WireCellUtil/Ress.h"
#include "WireCellUtil/IndexedSet.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Logging.h"

WIRECELL_FACTORY(BlobSolving, WireCell::Img::BlobSolving,
                 WireCell::INamed,
                 WireCell::IClusterFilter, WireCell::IConfigurable)

using namespace WireCell;

Img::BlobSolving::BlobSolving()
    : Aux::Logger("BlobSolving", "img")
{
}

Img::BlobSolving::~BlobSolving() {}

void Img::BlobSolving::configure(const WireCell::Configuration& cfg) {}

WireCell::Configuration Img::BlobSolving::default_configuration() const
{
    WireCell::Configuration cfg;
    return cfg;
}

static double measure_sum(const IChannel::shared_vector& imeas, const ISlice::pointer& islice)
{
    double value = 0;
    const auto& activity = islice->activity();
    for (const auto& ich : *(imeas.get())) {
        const auto& it = activity.find(ich);
        if (it == activity.end()) {
            continue;
        }
        value += it->second;
    }
    return value;
}

// Return a blob weight.  For now we hard code this but in the future
// this function can be replaced with a functional object slected
// based on configuration.
static double blob_weight(IBlob::pointer iblob, ISlice::pointer islice, const cluster_indexed_graph_t& grind)
{
    // magic numbers!
    const double default_weight = 9;
    const double reduction = 3;
    const size_t homer = 2;  // Max Power

    std::unordered_set<int> slices;
    IBlob::vector blobs = neighbors_oftype<IBlob::pointer>(grind, iblob);
    for (auto oblob : blobs) {
        for (auto oslice : neighbors_oftype<ISlice::pointer>(grind, oblob)) {
            slices.insert(oslice->ident());
        }
    }
    return default_weight / pow(reduction, std::min(homer, slices.size()));
}

static void solve_slice(cluster_indexed_graph_t& grind, ISlice::pointer islice)
{
    std::vector<std::vector<int> > b2minds;
    IndexedSet<IBlob::pointer> blobs;
    IndexedSet<IChannel::shared_vector> measures;
    for (IBlob::pointer iblob : neighbors_oftype<IBlob::pointer>(grind, islice)) {
        blobs(iblob);
        std::vector<int> minds;
        for (auto neigh : grind.neighbors(iblob)) {
            if (neigh.code() == 'm') {
                IChannel::shared_vector imeas = std::get<IChannel::shared_vector>(neigh.ptr);
                int mind = measures(imeas);
                minds.push_back(mind);
                continue;
            }
        }
        b2minds.push_back(minds);
    }

    const size_t nmeasures = measures.size();
    Ress::vector_t meas = Ress::vector_t::Zero(nmeasures);
    Ress::vector_t sigma = Ress::vector_t::Zero(nmeasures);
    for (size_t mind = 0; mind < nmeasures; ++mind) {
        const auto& imeas = measures.collection[mind];
        const double value = measure_sum(imeas, islice);
        if (value > 0.0) {
            const double sig = sqrt(value);  // Poisson uncertainty for now.
            sigma(mind) = sig;
            meas(mind) = value / sig;  // yes, this is just sig.
        }
    }

    const size_t nblobs = blobs.size();
    Ress::vector_t init = Ress::vector_t::Zero(nblobs);
    Ress::vector_t weight = Ress::vector_t::Zero(nblobs);
    Ress::matrix_t geom = Ress::matrix_t::Zero(nmeasures, nblobs);

    for (size_t bind = 0; bind < nblobs; ++bind) {
        IBlob::pointer iblob = blobs.collection[bind];
        init(bind) = iblob->value();
        weight(bind) = blob_weight(iblob, islice, grind);
        const auto& minds = b2minds[bind];
        for (int mind : minds) {
            const double sig = sigma(mind);
            if (sig > 0.0) {
                geom(mind, bind) = 1.0 / sig;
            }
        }
    }

    Ress::Params params;
    params.model = Ress::lasso;
    Ress::vector_t solved = Ress::solve(geom, meas, params, init, weight);

    for (size_t ind = 0; ind < nblobs; ++ind) {
        auto oblob = blobs.collection[ind];
        const double value = solved[ind];
        const double unc = 0.0;  // fixme, derive from solution + covariance
        IBlob::pointer nblob =
            std::make_shared<SimpleBlob>(oblob->ident(), value, unc, oblob->shape(), islice, oblob->face());
        grind.replace(oblob, nblob);
    }
}

bool Img::BlobSolving::operator()(const input_pointer& in, output_pointer& out)
{
    if (!in) {
        out = nullptr;
        log->debug("EOS");
        return true;
    }

    // start with a writable copy.
    cluster_indexed_graph_t grind(in->graph());
    for (auto islice : oftype<ISlice::pointer>(grind)) {
        solve_slice(grind, islice);
    }

    log->debug("send graph with {}",
               boost::num_vertices(grind.graph()));
                              
    out = std::make_shared<SimpleCluster>(grind.graph());
    return true;
}
