#ifndef WIRECELL_IFACE_ISLICEFANOUT
#define WIRECELL_IFACE_ISLICEFANOUT

#include "WireCellIface/IFanoutNode.h"
#include "WireCellIface/ISlice.h"

namespace WireCell {
    /** A slice fan-out component takes 1 input slice and produces one
     * slice on each of its output ports.  What each of those N slices
     * are depends on implementation.
     */
    class ISliceFanout : public IFanoutNode<ISlice, ISlice, 0> {
       public:
        virtual ~ISliceFanout();

        virtual std::string signature() { return typeid(ISliceFanout).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> output_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell

#endif
