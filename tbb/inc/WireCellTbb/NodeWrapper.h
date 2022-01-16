#ifndef WIRECELLTBB_NODEWRAPPER
#define WIRECELLTBB_NODEWRAPPER

#include "WireCellIface/INode.h"
#include "WireCellUtil/TupleHelpers.h"

#include <tbb/flow_graph.h>
#include <boost/any.hpp>
#include <utility>              // make_index_sequence
#include <memory>
#include <map>

namespace WireCellTbb {

    // Message type passed through WCT nodes
    using wct_t = boost::any;

    // We combine WCT data with a sequence number to assure order.
    using seqno_t = size_t;

    // The message type used by all TBB flow_graph nodes
    using msg_t = std::pair<seqno_t, wct_t>;
    using msg_vector_t = std::vector<msg_t>;

    // Broken out sender/receiver types, vectors of their pointers
    typedef tbb::flow::sender<msg_t> sender_type;
    typedef tbb::flow::receiver<msg_t> receiver_type;

    typedef std::vector<sender_type*> sender_port_vector;
    typedef std::vector<receiver_type*> receiver_port_vector;

    // Each WCT node is serviced by a small subgraph of TBB nodes.
    using src_node = tbb::flow::input_node<msg_t>;
    using func_node = tbb::flow::function_node<msg_t, msg_t>;
    using mfunc_node = tbb::flow::multifunction_node<msg_t, std::tuple<msg_t>>;
    using seq_node = tbb::flow::sequencer_node<msg_t>;
    using sink_node = tbb::flow::function_node<msg_t>;


    // A base facade which expose sender/receiver ports and provide
    // initialize hook.  There is one NodeWrapper for each node
    // category.
    class NodeWrapper {
       public:
        virtual ~NodeWrapper() {}

        virtual sender_port_vector sender_ports() { return sender_port_vector(); }
        virtual receiver_port_vector receiver_ports() { return receiver_port_vector(); }

        // call before running graph
        virtual void initialize() {}
    };

    // expose the wrappers only as a shared pointer
    typedef std::shared_ptr<NodeWrapper> Node;


    // tuple helpers

    template <typename Tuple, std::size_t... Is>
    msg_vector_t as_msg_vector(const Tuple& tup, std::index_sequence<Is...>)
    {
        return {std::get<Is>(tup)...};
    }
    template <typename Tuple>
    msg_vector_t as_msg_vector(const Tuple& tup)
    {
        return as_msg_vector(tup, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
    }

    template <typename Tuple, std::size_t... Is>
    receiver_port_vector receiver_ports(tbb::flow::join_node<Tuple>& jn, std::index_sequence<Is...>)
    {
        return {dynamic_cast<receiver_type*>(&tbb::flow::input_port<Is>(jn))...};
    }
    /// Return receiver ports of a join node as a vector.
    template <typename Tuple>
    receiver_port_vector receiver_ports(tbb::flow::join_node<Tuple>& jn)
    {
        return receiver_ports(jn, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
    }

    template <typename Tuple, std::size_t... Is>
    sender_port_vector sender_ports(tbb::flow::split_node<Tuple>& sp, std::index_sequence<Is...>)
    {
        return {dynamic_cast<sender_type*>(&tbb::flow::output_port<Is>(sp))...};
    }
    /// Return sender ports of a split node as a vector.
    template <typename Tuple>
    sender_port_vector sender_ports(tbb::flow::split_node<Tuple>& sp)
    {
        return sender_ports(sp, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
    }

}  // namespace WireCellTbb

#endif
