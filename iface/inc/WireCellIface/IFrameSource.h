#ifndef WIRECELL_IFRAMESOURCE
#define WIRECELL_IFRAMESOURCE

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/ISourceNode.h"
#include "WireCellIface/IFrame.h"

namespace WireCell {

    /** A frame source is something that generates IFrames.
     */
    class IFrameSource : public ISourceNode<IFrame> {
       public:
        typedef std::shared_ptr<IFrameSource> pointer;

        virtual ~IFrameSource();

        // supply:
        // virtual bool operator()(IFrame::pointer& frame);
    };

}  // namespace WireCell

#endif
