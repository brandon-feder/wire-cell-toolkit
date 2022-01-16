#ifndef WIRECELL_IFUNCTIONNODE
#define WIRECELL_IFUNCTIONNODE

#include "WireCellIface/INode.h"

#include <boost/any.hpp>
#include <vector>

namespace WireCell {

    /** A node which acts as a simple function.
     */
    class IFunctionNodeBase : public INode {
       public:
        typedef std::shared_ptr<IFunctionNodeBase> pointer;

        virtual ~IFunctionNodeBase();

        /// The calling signature:
        virtual bool operator()(const boost::any& anyin, boost::any& anyout) = 0;

        virtual NodeCategory category() { return functionNode; }
    };

    template <typename InputType, typename OutputType>
    class IFunctionNode : public IFunctionNodeBase {
       public:
        typedef InputType input_type;
        typedef OutputType output_type;
        typedef std::shared_ptr<const InputType> input_pointer;
        typedef std::shared_ptr<const OutputType> output_pointer;
        typedef IFunctionNode<InputType, OutputType> signature_type;

        virtual ~IFunctionNode() {}

        /// Set the signature for all subclasses.
        virtual std::string signature() { return typeid(signature_type).name(); }

        virtual bool operator()(const boost::any& anyin, boost::any& anyout)
        {
            const input_pointer& in = boost::any_cast<const input_pointer&>(anyin);
            output_pointer out;
            bool ok = (*this)(in, out);
            if (!ok) return false;
            anyout = out;
            return true;
        }

        /// The calling signature:
        virtual bool operator()(const input_pointer& in, output_pointer& out) = 0;

        // Return the names of the types this node takes as input.
        virtual std::vector<std::string> input_types() { return std::vector<std::string>{typeid(input_type).name()}; }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>{typeid(output_type).name()}; }
    };

}  // namespace WireCell

#endif
