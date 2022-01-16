#ifndef WIRECELL_IFRAMESINK
#define WIRECELL_IFRAMESINK

#include "WireCellIface/ISinkNode.h"
#include "WireCellIface/IFrame.h"

namespace WireCell {

    /** A frame sink is something that generates IFrames.
     */
    class IFrameSink : public ISinkNode<IFrame> {
       public:
        typedef std::shared_ptr<IFrameSink> pointer;

        virtual ~IFrameSink();

        virtual std::string signature() { return typeid(IFrameSink).name(); }

        // supply:
        // virtual bool operator()(const IFrame::pointer& frame);
    };

}  // namespace WireCell

#endif
