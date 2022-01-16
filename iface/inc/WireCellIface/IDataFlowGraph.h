#ifndef WIRECELL_IDATAFLOWGRAPH
#define WIRECELL_IDATAFLOWGRAPH

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/INode.h"

namespace WireCell {

    /** Interface to a data flow processing graph.
     *
     * See also WireCell::IConnector and WireCell::IConnectorT.
     */
    class IDataFlowGraph : public IComponent<IDataFlowGraph> {
       public:
        virtual ~IDataFlowGraph();

        /// Connect tail and head nodes so data runs from given tail
        /// port number to given head port number Return false on
        /// error.
        virtual bool connect(INode::pointer tail, INode::pointer head, size_t tail_port = 0, size_t head_port = 0) = 0;

        /// Run the graph, return false on error.
        virtual bool run() = 0;
    };

}  // namespace WireCell

#endif
