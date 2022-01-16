#include "WireCellUtil/DfpGraph.h"

#include <tuple>

using namespace WireCell;

DfpGraph::Vertex DfpGraph::get_add_vertex(const VertexProperty& tn)
{
    auto it = vertex_property_map.find(tn);
    if (it != vertex_property_map.end()) {
        return it->second;
    }
    auto v = add_vertex(tn, graph);
    vertex_property_map[tn] = v;
    return v;
}

DfpGraph::Edge DfpGraph::connect(const std::string& tail_type, const std::string& tail_name, int tail_port,
                                 const std::string& head_type, const std::string& head_name, int head_port)
{
    VertexProperty tvp(tail_type, tail_name);
    VertexProperty hvp(head_type, head_name);

    auto tv = get_add_vertex(tvp);
    auto hv = get_add_vertex(hvp);

    EdgeProperty ep(tail_port, head_port);
    Edge e;
    bool b;
    std::tie(e, b) = boost::add_edge(tv, hv, ep, graph);
    return e;
}

std::vector<DfpGraph::Connection> DfpGraph::connections()
{
    std::vector<Connection> ret;
    auto vits = boost::vertices(graph);
    for (auto v = vits.first; v != vits.second; ++v) {
        auto vp = graph[*v];
        auto eits = out_edges(*v, graph);

        if (eits.first == eits.second) continue;

        for (auto e = eits.first; e != eits.second; ++e) {
            auto ep = graph[*e];

            auto other = target(*e, graph);
            auto hp = graph[other];

            ret.push_back(std::make_tuple(vp, hp, ep));
        }
    }
    return ret;
}

static std::tuple<std::string, std::string, int> get_node(WireCell::Configuration jone)
{
    using namespace WireCell;
    auto node = jone["node"].asString();
    auto tn = String::split(node, ":");
    std::string type = "";
    std::string name = "";
    if (tn.size() == 2) {
        type = tn[0];
        name = tn[1];
    }
    else if (tn.size() == 1) {
        type = tn[0];
    }

    int port = get(jone, "port", 0);

    return {type, name, port};
}

void DfpGraph::configure(const Configuration& cfg)
{
    for (auto conn : cfg) {
        auto head = get_node(conn["head"]);
        auto tail = get_node(conn["tail"]);

        connect(std::get<0>(tail), std::get<1>(tail), std::get<2>(tail), std::get<0>(head), std::get<1>(head),
                std::get<2>(head));
    }
}
