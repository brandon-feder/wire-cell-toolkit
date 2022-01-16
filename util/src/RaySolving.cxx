#include "WireCellUtil/RaySolving.h"
#include "WireCellUtil/Ress.h"

using namespace WireCell;

RayGrid::Grouping::vertex_t RayGrid::Grouping::wire_node(ident_t wid)
{
    auto it = m_wid2vtx.find(wid);
    if (it != m_wid2vtx.end()) {
        return it->second;
    }
    vertex_t node = boost::add_vertex(m_graph);
    m_wid2vtx[wid] = node;
    m_graph[node].ntype = 'w';
    m_graph[node].ident = wid;
    m_graph[node].value = 0.0;
    m_graph[node].weight = 1.0;
    return node;
}

void RayGrid::Grouping::add(char ntype, ident_t chid, std::vector<ident_t> wids, float value, float weight)
{
    vertex_t node = boost::add_vertex(m_graph);
    m_graph[node].ntype = ntype;
    m_graph[node].ident = chid;
    m_graph[node].value = value;
    m_graph[node].weight = weight;

    for (const auto wid : wids) {
        auto wnode = wire_node(wid);
        boost::add_edge(node, wnode, m_graph);
    }
}

RayGrid::Grouping::clusterset_t RayGrid::Grouping::clusters()
{
    clusterset_t ret;

    std::unordered_map<vertex_t, int> stripes;
    int nstripes = boost::connected_components(m_graph, boost::make_assoc_property_map(stripes));
    if (!nstripes) {
        return ret;
    }

    for (auto& vci : stripes) {
        vertex_t vtx = vci.first;
        char ntype = m_graph[vtx].ntype;
        if (ntype == 'w') {
            continue;
        }
        const size_t stripe_index = vci.second;
        ret[stripe_index].push_back(m_graph[vtx]);
    }
    return ret;
}

//
// Solving
//

RayGrid::Solving::vertex_t RayGrid::Solving::source_node(ident_t sid, float value, float weight)
{
    auto it = m_sid2vtx.find(sid);
    if (it != m_sid2vtx.end()) {
        return it->second;
    }
    vertex_t snode = boost::add_vertex(m_graph);
    m_sid2vtx[sid] = snode;
    m_graph[snode].ntype = 's';
    m_graph[snode].ident = sid;
    m_graph[snode].value = value;
    m_graph[snode].weight = weight;
    return snode;
}

RayGrid::Solving::vertex_t RayGrid::Solving::measurement_node(float value, float weight)
{
    vertex_t mnode = boost::add_vertex(m_graph);
    m_graph[mnode].ntype = 'm';
    m_graph[mnode].ident = 0xdeadbeef;
    m_graph[mnode].value = value;
    m_graph[mnode].weight = weight;
    return mnode;
}

void RayGrid::Solving::add(const Grouping::clusterset_t& cset)
{
    for (const auto& it : cset) {
        double total_value = 0;
        double total_weight = 0;
        int nms = 0;
        std::vector<vertex_t> snodes;
        for (const auto& node : it.second) {
            if (node.ntype == 'm') {
                total_value += node.value;
                total_weight += node.weight;
                ++nms;
                continue;
            }
            if (node.ntype == 's') {
                auto snode = source_node(node.ident, node.value, node.weight);
                snodes.push_back(snode);
                continue;
            }
            // skip w's
        }
        if (!nms or snodes.empty()) {
            // require at least one measurement and one source.
            continue;
        }
        auto mnode = measurement_node(total_value, total_weight / nms);
        for (auto snode : snodes) {
            boost::add_edge(mnode, snode, m_graph);
        }
    }
}

void RayGrid::Solving::solve_one(solution_t& answer, const std::vector<vertex_t>& sources,
                                 const std::vector<vertex_t>& measures)
{
    // convert source nodes and their edge end mnodes to m vector and G matrix
    Ress::vector_t meas = Ress::vector_t::Zero(measures.size());
    Ress::vector_t init = Ress::vector_t::Zero(sources.size());
    Ress::vector_t weight = Ress::vector_t::Zero(sources.size());
    Ress::matrix_t geom = Ress::matrix_t::Zero(measures.size(), sources.size());
    std::unordered_map<vertex_t, size_t> sv2ind, mv2ind;
    for (size_t mind = 0; mind < measures.size(); ++mind) {
        vertex_t mvtx = measures[mind];
        mv2ind[mvtx] = mind;
        meas(mind) = m_graph[mvtx].value;
    }
    for (size_t sind = 0; sind < sources.size(); ++sind) {
        vertex_t svtx = sources[sind];
        sv2ind[svtx] = sind;
        init(sind) = m_graph[svtx].value;
        weight(sind) = m_graph[svtx].weight;
        auto medges = boost::out_edges(svtx, m_graph);
        for (auto mit = medges.first; mit != medges.second; ++mit) {
            auto mvtx = target(*mit, m_graph);
            size_t mind = mv2ind[mvtx];
            geom(mind, sind) = 1.0;
        }
        // fixme: this ignores measurement weights.
    }

    Ress::Params params;
    params.model = Ress::lasso;
    Ress::vector_t solved = Ress::solve(geom, meas, params, init, weight);
    for (int sind = 0; sind < solved.size(); ++sind) {
        auto svtx = sources[sind];
        m_graph[svtx].value = solved[sind];
        answer[m_graph[svtx].ident] = solved[sind];
    }
}

RayGrid::Solving::solution_t RayGrid::Solving::solve()
{
    std::unordered_map<vertex_t, int> probs;
    int nprobs = boost::connected_components(m_graph, boost::make_assoc_property_map(probs));

    solution_t answer;
    if (!nprobs) {
        return answer;
    }

    struct SMVecs {
        std::vector<vertex_t> sources, measures;
    };
    std::unordered_map<int, SMVecs> smvecs;
    for (auto it : probs) {
        auto vtx = it.first;
        if (m_graph[vtx].ntype == 's') {
            smvecs[it.second].sources.push_back(vtx);
        }
        if (m_graph[vtx].ntype == 'm') {
            smvecs[it.second].measures.push_back(vtx);
        }
    }

    for (auto sit : smvecs) {
        solve_one(answer, sit.second.sources, sit.second.measures);
    }
    return answer;
}
