#ifndef WIRECELL_ISLICESTRIPPER
#define WIRECELL_ISLICESTRIPPER

#include "WireCellIface/ISlice.h"
#include "WireCellIface/IStripeSet.h"
#include "WireCellIface/IFunctionNode.h"

namespace WireCell {

    /** A slice striper produces a set of stripes from a slice.
     */
    class ISliceStriper : public IFunctionNode<ISlice, IStripeSet> {
       public:
        typedef std::shared_ptr<ISliceStriper> pointer;

        virtual ~ISliceStriper();

        virtual std::string signature() { return typeid(ISliceStriper).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };
}  // namespace WireCell

#endif
