#ifndef WIRECELL_ISPLITNODE
#define WIRECELL_ISPLITNODE

#include "WireCellIface/INode.h"

#include "WireCellUtil/TupleHelpers.h"

#include <boost/any.hpp>
#include <vector>
#include <memory>

namespace WireCell {

    /** A node which splits 1 data into N objects each of a distinct
     * type.
     */

    class ISplitNodeBase : public INode {
       public:
        typedef std::shared_ptr<ISplitNodeBase> pointer;

        virtual ~ISplitNodeBase();

        typedef std::vector<boost::any> any_vector;

        /// The calling signature:
        virtual bool operator()(const boost::any& anyin, any_vector& anyout) = 0;

        virtual NodeCategory category() { return splitNode; }
    };

    //
    template <typename InputType, typename OutputTuple>
    class ISplitNode : public ISplitNodeBase {
       public:
        typedef tuple_helper<OutputTuple> port_helper_type;
        typedef typename port_helper_type::template WrappedConst<std::shared_ptr>::type output_tuple_type;
        typedef tuple_helper<output_tuple_type> output_helper_type;

        typedef InputType input_type;

        typedef std::shared_ptr<const InputType> input_pointer;

        virtual ~ISplitNode() {}

        virtual bool operator()(const boost::any& anyin, any_vector& anyvout)
        {
            const input_pointer& in = boost::any_cast<const input_pointer&>(anyin);

            output_tuple_type outtup;
            output_helper_type oh;

            bool ok = (*this)(in, outtup);
            if (ok) {
                anyvout = oh.as_any(outtup);
            }
            return ok;
        }

        virtual bool operator()(const input_pointer& in, output_tuple_type& out) = 0;

        // Return the names of the types this node takes as input.
        virtual std::vector<std::string> input_types() { return std::vector<std::string>{typeid(input_type).name()}; }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types()
        {
            port_helper_type oph;
            return oph.type_names();
        }
    };

}  // namespace WireCell

#endif
