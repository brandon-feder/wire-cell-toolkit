#ifndef WIRECELL_IQUEUEDOUTNODE
#define WIRECELL_IQUEUEDOUTNODE

#include "WireCellIface/INode.h"

#include <boost/any.hpp>
#include <deque>
#include <vector>

namespace WireCell {

    /** A node which is a function producing a zero or more output
     */
    class IQueuedoutNodeBase : public INode {
       public:
        typedef std::shared_ptr<IQueuedoutNodeBase> pointer;

        virtual ~IQueuedoutNodeBase();

        typedef std::deque<boost::any> queuedany;

        /// The calling signature:
        virtual bool operator()(const boost::any& anyin, queuedany& out) = 0;

        virtual NodeCategory category() { return queuedoutNode; }
    };

    template <typename InputType, typename OutputType>
    class IQueuedoutNode : public IQueuedoutNodeBase {
       public:
        typedef std::shared_ptr<IQueuedoutNodeBase> pointer;

        typedef InputType input_type;
        typedef OutputType output_type;
        typedef std::shared_ptr<const InputType> input_pointer;
        typedef std::shared_ptr<const OutputType> output_pointer;
        typedef std::deque<output_pointer> output_queue;

        virtual ~IQueuedoutNode() {}

        virtual bool operator()(const boost::any& anyin, queuedany& outanyq)
        {
            const input_pointer& in = boost::any_cast<const input_pointer&>(anyin);
            output_queue outq;
            bool ok = (*this)(in, outq);
            if (!ok) return false;
            for (auto o : outq) {  // transfer typed to any
                outanyq.push_back(o);
            }
            return true;
        }

        /// The calling signature:
        virtual bool operator()(const input_pointer& in, output_queue& outq) = 0;

        // Return the names of the types this node takes as input.
        virtual std::vector<std::string> input_types() { return std::vector<std::string>{typeid(input_type).name()}; }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>{typeid(output_type).name()}; }
    };

}  // namespace WireCell

#endif
