#ifndef WIRECELL_PGRAPH_FACTORY
#define WIRECELL_PGRAPH_FACTORY

#include "WireCellPgraph/Node.h"
#include "WireCellUtil/Logging.h"
#include "WireCellIface/INode.h"

namespace WireCell {
    namespace Pgraph {

        // Makers make an appropriate Pgraph::Node from an INode
        struct Maker {
            virtual ~Maker() {}
            virtual Node* operator()(INode::pointer wcnode) = 0;
        };
        template <class Wrapper>
        struct MakerT : public Maker {
            virtual ~MakerT() {}
            virtual Node* operator()(INode::pointer wcnode) { return new Wrapper(wcnode); }
        };

        // This factory creates Pgraph::Nodes*'s from INode::pointers.
        // It only knows about the categories that have been hard
        // coded in the Factory constructor.
        class Factory {
           public:
            Factory();

            typedef std::map<INode::pointer, Node*> WCNodeWrapperMap;

            template <class Wrapper>
            void bind_maker(INode::NodeCategory cat)
            {
                m_factory[cat] = new MakerT<Wrapper>;
            }

            Node* operator()(WireCell::INode::pointer wcnode);

           private:
            typedef std::map<INode::NodeCategory, Maker*> NodeMakers;
            NodeMakers m_factory;
            WCNodeWrapperMap m_nodes;
            Log::logptr_t l;
        };

    }  // namespace Pgraph
}  // namespace WireCell
#endif
