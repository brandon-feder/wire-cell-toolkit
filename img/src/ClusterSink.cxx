#include "WireCellImg/ClusterSink.h"
#include <boost/graph/graphviz.hpp>

#include "WireCellUtil/String.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Logging.h"

#include <fstream>
#include <sstream>
#include <functional>
#include <unordered_set>

WIRECELL_FACTORY(ClusterSink, WireCell::Img::ClusterSink, WireCell::IClusterSink, WireCell::IConfigurable)

using namespace WireCell;

Img::ClusterSink::ClusterSink()
  : m_filename("")
  , m_node_types("bsm")
{
}

Img::ClusterSink::~ClusterSink() {}

void Img::ClusterSink::configure(const WireCell::Configuration& cfg)
{
    m_filename = get(cfg, "filename", m_filename);
    m_node_types = get(cfg, "node_types", m_node_types);
}

WireCell::Configuration Img::ClusterSink::default_configuration() const
{
    WireCell::Configuration cfg;
    cfg["filename"] = m_filename;

    // A string with any of characters "wcbsm".  Including wires and
    // channels tends to make a crazy big plot so by default they are
    // excluded.
    cfg["node_types"] = m_node_types;
    return cfg;
}

typedef std::vector<std::function<std::string(const cluster_node_t& ptr)> > stringers_t;

static std::string size_stringer(const cluster_node_t& n)
{
    size_t sp = std::get<size_t>(n.ptr);
    std::stringstream ss;
    ss << n.code() << ":" << sp;
    return ss.str();
}

template <typename Type>
std::string scalar_stringer(const cluster_node_t& n)
{
    typename Type::pointer sp = std::get<typename Type::pointer>(n.ptr);
    std::stringstream ss;
    ss << n.code() << ":" << sp->ident();
    return ss.str();
}
template <typename Type>
std::string vector_stringer(const cluster_node_t& n)
{
    typename Type::shared_vector sv = std::get<typename Type::shared_vector>(n.ptr);
    std::stringstream ss;
    ss << n.code() << "#" << sv->size();
    return ss.str();
}

static std::string asstring(const cluster_node_t& n)
{
    // cwbsm
    stringers_t ss{size_stringer,          scalar_stringer<IChannel>, scalar_stringer<IWire>,
                   scalar_stringer<IBlob>, scalar_stringer<ISlice>,   vector_stringer<IChannel>};
    const size_t ind = n.ptr.index();
    return ss[ind](n);
}

struct label_writer_t {
    const cluster_graph_t& g;
    template <class vdesc_t>
    void operator()(std::ostream& out, const vdesc_t v) const
    {
        out << "[label=\"" << asstring(g[v]) << "\"]";
    }
};

bool Img::ClusterSink::operator()(const ICluster::pointer& cluster)
{
    if (!cluster) {
        return true;
    }

    std::string fname = m_filename;
    if (fname.empty()) {
        return true;
    }

    if (m_filename.find("%") != std::string::npos) {
        fname = String::format(m_filename, cluster->ident());
    }
    std::ofstream out(fname.c_str());
    spdlog::info("Writing graphviz to {}", fname);

    std::unordered_set<char> keep(m_node_types.begin(), m_node_types.end());

    // use indexed graph basically just for the copy()
    const cluster_graph_t& gr = cluster->graph();
    cluster_indexed_graph_t grind;

    for (const auto& v : boost::make_iterator_range(boost::vertices(gr))) {
        const auto& vobj = gr[v];
        if (!keep.count(vobj.code())) {
            continue;
        }
        grind.vertex(vobj);
        for (auto edge : boost::make_iterator_range(boost::out_edges(v, gr))) {
            auto v2 = boost::target(edge, gr);
            const auto& vobj2 = gr[v2];
            if (keep.count(vobj2.code())) {
                grind.edge(vobj, vobj2);
            }
        }
    }
    const cluster_graph_t& gr2 = grind.graph();
    boost::write_graphviz(out, gr2, label_writer_t{gr2});

    return true;
}
