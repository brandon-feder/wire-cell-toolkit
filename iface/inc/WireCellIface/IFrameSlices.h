#ifndef WIRECELL_IFRAMESLICES
#define WIRECELL_IFRAMESLICES

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/ISlice.h"

namespace WireCell {

    /** A frame slicer conceptually performs a transpose of a frame
     * from being tick-major order to being channel-major while on the
     * way possibly rebinning the per-tick samples by applying some
     * metric function (typically just a sum) and possibly some
     * uncertainty function.
     *
     * See also ISliceFrame which is a more monolithic but function node.
     */
    class IFrameSlices : public IQueuedoutNode<IFrame, ISlice> {
       public:
        typedef std::shared_ptr<IFrameSlices> pointer;

        virtual ~IFrameSlices();

        virtual std::string signature() { return typeid(IFrameSlices).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
