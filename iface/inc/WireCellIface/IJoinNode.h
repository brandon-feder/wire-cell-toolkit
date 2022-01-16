#ifndef WIRECELL_IJOINNODE
#define WIRECELL_IJOINNODE

#include "WireCellIface/INode.h"

#include "WireCellUtil/TupleHelpers.h"

#include <boost/any.hpp>
#include <vector>
#include <memory>

namespace WireCell {

    /** A node which joins N data objects each of a distinct type and
     * arriving synchronously to produce a single output object.
     */

    class IJoinNodeBase : public INode {
       public:
        typedef std::shared_ptr<IJoinNodeBase> pointer;

        virtual ~IJoinNodeBase();

        typedef std::vector<boost::any> any_vector;

        /// The calling signature:
        virtual bool operator()(const any_vector& anyin, boost::any& anyout) = 0;

        virtual NodeCategory category() { return joinNode; }
    };

    //
    template <typename InputTuple, typename OutputType>
    class IJoinNode : public IJoinNodeBase {
       public:
        typedef tuple_helper<InputTuple> port_helper_type;
        typedef typename port_helper_type::template WrappedConst<std::shared_ptr>::type input_tuple_type;
        typedef tuple_helper<input_tuple_type> input_helper_type;

        typedef OutputType output_type;

        typedef std::shared_ptr<const OutputType> output_pointer;

        virtual ~IJoinNode() {}

        virtual bool operator()(const any_vector& anyv, boost::any& anyout)
        {
            input_helper_type ih;
            auto intup = ih.from_any(anyv);

            output_pointer out;
            bool ok = (*this)(intup, out);
            if (ok) {
                anyout = out;
            }
            return ok;
        }

        virtual bool operator()(const input_tuple_type& intup, output_pointer& out) = 0;

        // Return the names of the types this node takes as input.
        virtual std::vector<std::string> input_types()
        {
            port_helper_type iph;
            return iph.type_names();
        }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>{typeid(output_type).name()}; }
    };

}  // namespace WireCell

#endif
