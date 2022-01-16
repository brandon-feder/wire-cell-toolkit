#ifndef WIRECELL_IFANINNODE
#define WIRECELL_IFANINNODE

#include "WireCellIface/INode.h"

#include <boost/any.hpp>
#include <vector>

namespace WireCell {

    /** A node which fans-in N data objects of the same, given
     * InputType to produce the given OutputType.
     */

    class IFaninNodeBase : public INode {
       public:
        typedef std::shared_ptr<IFaninNodeBase> pointer;

        virtual ~IFaninNodeBase();

        typedef std::vector<boost::any> any_vector;

        /// The calling signature:
        virtual bool operator()(const any_vector& anyin, boost::any& anyout) = 0;

        virtual NodeCategory category() { return faninNode; }
    };

    // This converts between any and typed.
    template <typename InputType, typename OutputType, int FaninMultiplicity = 3>
    class IFaninNode : public IFaninNodeBase {
       public:
        typedef InputType input_type;
        typedef OutputType output_type;
        typedef std::shared_ptr<const InputType> input_pointer;
        typedef std::shared_ptr<const OutputType> output_pointer;
        typedef std::vector<input_pointer> input_vector;

        virtual ~IFaninNode() {}

        virtual bool operator()(const any_vector& anyv, boost::any& anyout)
        {
            input_vector invec;
            for (auto a : anyv) {
                auto in = boost::any_cast<input_pointer>(a);
                invec.push_back(in);
            }
            output_pointer out;
            bool ok = (*this)(invec, out);
            if (ok) {
                anyout = out;
            }
            return ok;
        }

        // The typed interface
        virtual bool operator()(const input_vector& invec, output_pointer& out) = 0;

        // Return the names of the types this node takes as input.
        // Note: if subclass wants to supply FaninMultiplicity at
        // construction time, this needs to be overridden.
        virtual std::vector<std::string> input_types()
        {
            std::vector<std::string> ret(FaninMultiplicity, std::string(typeid(input_type).name()));
            return ret;
        }
        // Return the names of the types this node produces as output.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>{typeid(output_type).name()}; }
    };

}  // namespace WireCell

#endif
