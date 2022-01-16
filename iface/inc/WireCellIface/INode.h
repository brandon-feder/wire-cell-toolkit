
#ifndef WIRECELL_INODE
#define WIRECELL_INODE

#include "WireCellUtil/IComponent.h"

#include <boost/any.hpp>

#include <memory>
#include <vector>

namespace WireCell {

    /** A data flow node
     */
    class INode : public IComponent<INode> {
       public:
        virtual ~INode();

        enum NodeCategory {
            unknown,
            sourceNode,     // one pointer output
            sinkNode,       // one pointer input
            functionNode,   // one pointer input / output
            queuedoutNode,  // one pointer input, queue of output
            joinNode,       // tuple input, pointer output
            splitNode,      // pointer input, tuple output
            faninNode,      // vector input, pointer output
            fanoutNode,     // pointer input, vector output
            multioutNode,   // pointer input multi-queue output
            hydraNode,      // multi-queue input and output
        };

        /// Return the behavior category type
        virtual NodeCategory category() = 0;

        // fixme: I can probably remove this from the API.
        /// The signature is string unique to all classes that
        /// implement a particular calling signature.  These should be
        /// defined in lower level interfaces such as a mythical
        /// IMyFooToBarConverter.
        virtual std::string signature() = 0;

        // Subclasses may override to provide the number of instances
        // that can run concurrently.  Default is 1.  Return 0 for
        // unlimited.  It is expected that NO node has concurency
        // other than unity for two reasons: 1) The node itself must
        // not have mutable data which tends not to be possible.  2)
        // Concurent running of the same node violates fairly
        // pervasive ordering assumptions for downstreadm nodes.  If
        // you think your node should run concurrently with itself,
        // test carefully with full graphs and with TbbFlow given more
        // than one thread.  See #121 for some more details.
        virtual int concurrency() { return 1; }

        // Return string representations of the C++ types this node takes as input.
        // When a node is used in a DFP graph, these enumerate the input ports.
        virtual std::vector<std::string> input_types() { return std::vector<std::string>(); }
        // Return string representations of the C++ types this node produces as output.
        // When a node is used in a DFP graph, these enumerate the output ports.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>(); }

        /// Optional hook to be implemented in order to reset after an
        /// end of stream is encountered.  Fixme: this should be removed.
        virtual void reset() {}
    };
}  // namespace WireCell

#endif
