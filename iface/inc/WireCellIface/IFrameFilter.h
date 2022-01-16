#ifndef WIRECELL_IFRAMEFILTER
#define WIRECELL_IFRAMEFILTER

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IFrame.h"

namespace WireCell {

    /** A frame filter is something that applies some transformation
     * on its input frame to produce and output frame.  This is a
     * functional node so does no buffering.  The unit of the sample
     * of the output frame may differ from input.
     *
     * Note, if the output frame samples are conceptually integral
     * they are nonetheless still stored as floating point values.
     * Consumers of the output frame should take care of rounding and
     * truncating as required.
     */
    class IFrameFilter : public IFunctionNode<IFrame, IFrame> {
       public:
        typedef std::shared_ptr<IFrameFilter> pointer;

        virtual ~IFrameFilter();

        virtual std::string signature() { return typeid(IFrameFilter).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
