#ifndef WIRECELL_IFACE_IFRAMEFANOUT
#define WIRECELL_IFACE_IFRAMEFANOUT

#include "WireCellIface/IFanoutNode.h"
#include "WireCellIface/IFrame.h"

namespace WireCell {
    /** A frame fan-out component takes 1 input frame and produces one
     * frame on each of its output ports.  What each of those N frames
     * are depends on implementation.

     */
    class IFrameFanout : public IFanoutNode<IFrame, IFrame, 0> {
       public:
        virtual ~IFrameFanout();

        virtual std::string signature() { return typeid(IFrameFanout).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> output_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell

#endif
