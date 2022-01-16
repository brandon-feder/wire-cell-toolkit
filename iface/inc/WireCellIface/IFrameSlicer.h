#ifndef WIRECELL_IFRAMESLICER
#define WIRECELL_IFRAMESLICER

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/ISliceFrame.h"

namespace WireCell {

    /** A frame slicer conceptually performs a transpose of a frame
     * from being tick-major order to being channel-major while on the
     * way possibly rebinning the per-tick samples by applying some
     * metric function (typically just a sum) and possibly some
     * uncertainty function.
     *
     * Note, there could be a closely related IQueuedoutNode which
     * instead returns a queue of ISlice which might be more suited to
     * stream processing.  OTOH, implementations of this IFrameSlicer
     * are expected produce an ISliceFrame which spans the same time
     * as the input IFrame.
     */
    class IFrameSlicer : public IFunctionNode<IFrame, ISliceFrame> {
       public:
        typedef std::shared_ptr<IFrameSlicer> pointer;

        virtual ~IFrameSlicer();

        virtual std::string signature() { return typeid(IFrameSlicer).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
