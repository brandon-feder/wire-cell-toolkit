/** A "cluster" (as defined here) is a graph connecting shared points
 * to instances of a specific set of types of WCT data classes related
 * to WC imaging.  See node_t below for the supported types.
 */

#ifndef WIRECELL_ICLUSTER
#define WIRECELL_ICLUSTER

#include "WireCellIface/IChannel.h"
#include "WireCellIface/IWire.h"
#include "WireCellIface/IBlob.h"
#include "WireCellIface/ISlice.h"
#include "WireCellIface/WirePlaneId.h"

#include "WireCellUtil/IndexedGraph.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <variant>

namespace WireCell {

    /// The vertex property.
    typedef std::variant<size_t, IChannel::pointer, IWire::pointer, IBlob::pointer, ISlice::pointer,
                         IChannel::shared_vector>
        cluster_ptr_t;

    struct cluster_node_t {
        cluster_ptr_t ptr;

        cluster_node_t()
          : ptr()
        {
        }
        cluster_node_t(const cluster_ptr_t& p)
          : ptr(p)
        {
        }
        cluster_node_t(const IChannel::pointer& p)
          : ptr(p)
        {
        }
        cluster_node_t(const IWire::pointer& p)
          : ptr(p)
        {
        }
        cluster_node_t(const IBlob::pointer& p)
          : ptr(p)
        {
        }
        cluster_node_t(const ISlice::pointer& p)
          : ptr(p)
        {
        }
        cluster_node_t(const IChannel::shared_vector& p)
          : ptr(p)
        {
        }

        cluster_node_t(const cluster_node_t& other)
          : ptr(other.ptr)
        {
        }

        // Helper: return a letter code for the type of the ptr or \0.
        char code() const
        {
            auto ind = ptr.index();
            if (ind == std::variant_npos) {
                return 0;
            }
            return "0cwbsm"[ind];
        }
        bool operator==(const cluster_node_t& other) const { return ptr == other.ptr; }
        cluster_node_t& operator=(const cluster_node_t& other)
        {
            ptr = other.ptr;
            return *this;
        }
    };
}  // namespace WireCell

namespace std {
    template <>
    struct hash<WireCell::cluster_node_t> {
        std::size_t operator()(const WireCell::cluster_node_t& n) const
        {
            size_t h = 0;
            switch (n.ptr.index()) {
            case 0:
                h = std::get<0>(n.ptr);
                break;
            case 1:
                h = (std::size_t) std::get<1>(n.ptr).get();
                break;
            case 2:
                h = (std::size_t) std::get<2>(n.ptr).get();
                break;
            case 3:
                h = (std::size_t) std::get<3>(n.ptr).get();
                break;
            case 4:
                h = (std::size_t) std::get<4>(n.ptr).get();
                break;
            case 5:
                h = (std::size_t) std::get<5>(n.ptr).get();
                break;
            }
            return h;
        }
    };
}  // namespace std

namespace WireCell {

    typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, cluster_node_t> cluster_graph_t;
    typedef boost::graph_traits<cluster_graph_t>::vertex_descriptor cluster_vertex_t;
    typedef boost::graph_traits<cluster_graph_t>::edge_descriptor cluster_edge_t;
    typedef boost::graph_traits<cluster_graph_t>::vertex_iterator cluster_vertex_iter_t;

    class ICluster : public IData<ICluster> {
       public:
        virtual ~ICluster();

        /// Return an identifying number.
        virtual int ident() const = 0;

        // Access the graph.
        virtual const cluster_graph_t& graph() const = 0;
    };

    typedef IndexedGraph<cluster_node_t> cluster_indexed_graph_t;

    template <typename Type>
    std::vector<Type> oftype(const cluster_indexed_graph_t& g)
    {
        std::vector<Type> ret;
        for (const auto& v : boost::make_iterator_range(boost::vertices(g.graph()))) {
            const auto& vp = g.graph()[v];
            if (std::holds_alternative<Type>(vp.ptr)) {
                ret.push_back(std::get<Type>(vp.ptr));
            }
        }
        return ret;
    }

    template <typename Type>
    std::vector<Type> neighbors_oftype(const cluster_indexed_graph_t& g, const cluster_node_t& n)
    {
        std::vector<Type> ret;
        for (const auto& vp : g.neighbors(n)) {
            if (std::holds_alternative<Type>(vp.ptr)) {
                ret.push_back(std::get<Type>(vp.ptr));
            }
        }
        return ret;
    }

}  // namespace WireCell

#endif
