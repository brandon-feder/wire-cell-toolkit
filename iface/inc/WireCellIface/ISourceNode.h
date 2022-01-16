#ifndef WIRECELL_ISOURCENODE
#define WIRECELL_ISOURCENODE

#include "WireCellIface/INode.h"

#include <boost/any.hpp>
#include <vector>

namespace WireCell {

    /** A node which acts as a source.
     */
    class ISourceNodeBase : public INode {
       public:
        typedef std::shared_ptr<ISourceNodeBase> pointer;

        virtual ~ISourceNodeBase();

        virtual NodeCategory category() { return sourceNode; }

        virtual bool operator()(boost::any& anyout) = 0;
    };

    template <typename OutputType>
    class ISourceNode : public ISourceNodeBase {
       public:
        typedef OutputType output_type;

        typedef ISourceNode<OutputType> signature_type;
        typedef std::shared_ptr<signature_type> pointer;

        typedef std::shared_ptr<const OutputType> output_pointer;

        virtual ~ISourceNode() {}

        virtual NodeCategory category() { return sourceNode; }

        /// Set the signature for all subclasses.
        virtual std::string signature() { return typeid(signature_type).name(); }

        virtual bool operator()(boost::any& anyout)
        {
            output_pointer out;
            bool ok = (*this)(out);
            if (!ok) return false;
            anyout = out;

            return true;
        }

        /// The calling signature:
        virtual bool operator()(output_pointer& out) = 0;

        // Return the names of the types this node takes as output.
        virtual std::vector<std::string> output_types() { return std::vector<std::string>{typeid(output_type).name()}; }
    };

}  // namespace WireCell

#endif
