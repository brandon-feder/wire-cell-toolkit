#ifndef WIRECELLTBB_WRAPPERFACTORY
#define WIRECELLTBB_WRAPPERFACTORY

#include "WireCellIface/INode.h"
#include "WireCellTbb/NodeWrapper.h"

namespace WireCellTbb {

    // Make a node wrapper for every type of node category
    struct WrapperMaker {
        virtual ~WrapperMaker() {}
        virtual Node operator()(tbb::flow::graph& g, WireCell::INode::pointer n) = 0;
    };
    template <class Wrapper>
    struct WrapperMakerT : public WrapperMaker {
        virtual ~WrapperMakerT() {}
        virtual Node operator()(tbb::flow::graph& g, WireCell::INode::pointer n) { return Node(new Wrapper(g, n)); }
    };

    class WrapperFactory {
       public:
        WrapperFactory(tbb::flow::graph& graph);

        template <class Wrapper>
        void bind_maker(WireCell::INode::NodeCategory cat)
        {
            m_factory[cat] = new WrapperMakerT<Wrapper>;
        }

        Node operator()(WireCell::INode::pointer wcnode);

        typedef std::map<WireCell::INode::pointer, Node> WCNode2Wrapper;
        WCNode2Wrapper& seen() { return m_nodes; }

       private:
        typedef std::map<WireCell::INode::NodeCategory, WrapperMaker*> NodeMakers;

        tbb::flow::graph& m_graph;
        NodeMakers m_factory;
        WCNode2Wrapper m_nodes;
    };

}  // namespace WireCellTbb

#endif
