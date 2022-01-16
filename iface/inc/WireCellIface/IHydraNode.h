/** A hydra has N input queues and M output queues each of a specified
 * type.  Not all queues may have elements.  The implementation
 * callable may leave elements in input queues for subsequent calls
 * when new data exists.
 */

#ifndef WIRECELL_IHYDRANODE
#define WIRECELL_IHYDRANODE

#include "WireCellIface/INode.h"
#include "WireCellUtil/TupleHelpers.h"

#include <boost/any.hpp>
#include <vector>
#include <deque>

namespace WireCell {

    /** Base hydra node class.
     *
     */
    class IHydraNodeBase : public INode {
       public:
        typedef std::shared_ptr<IHydraNodeBase> pointer;

        virtual ~IHydraNodeBase();

        typedef std::deque<boost::any> any_queue;
        typedef std::vector<any_queue> any_queue_vector;

        /// The calling signature:
        virtual bool operator()(any_queue_vector& anyinq, any_queue_vector& anyoutq) = 0;

        virtual NodeCategory category() { return hydraNode; }
    };

    /** A hydra with input and output fixed at compile-time with tuples.
     */
    template <typename InputTuple, typename OutputTuple>
    class IHydraNode : public IHydraNodeBase {
       public:
        typedef InputTuple input_tuple_type;
        typedef OutputTuple output_tuple_type;

        typedef shared_queued<InputTuple> input_shqed;
        typedef shared_queued<OutputTuple> output_shqed;

        typedef typename input_shqed::shared_queued_tuple_type input_queues_type;
        typedef typename output_shqed::shared_queued_tuple_type output_queues_type;

        virtual ~IHydraNode() {}

        /// Translate call from any to types and back.
        virtual bool operator()(any_queue_vector& anyinq, any_queue_vector& anyoutq)
        {
            input_shqed ih;
            output_shqed oh;

            auto inq = ih.from_any_queue(anyinq);
            output_queues_type outq;

            bool ok = (*this)(inq, outq);
            if (ok) {
                anyoutq = oh.as_any_queue(outq);
            }

            // (re)set input queue
            anyinq = ih.as_any_queue(inq);
            return ok;
        }

        /// Typed interface for subclass to implement.
        virtual bool operator()(input_queues_type& inqs, output_queues_type& outqs) = 0;

        // Return the names of the types this node takes as input.
        virtual std::vector<std::string> input_types()
        {
            tuple_helper<input_tuple_type> ih;
            return ih.type_names();
        }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types()
        {
            tuple_helper<output_tuple_type> oh;
            return oh.type_names();
        }
    };

}  // namespace WireCell

#endif
