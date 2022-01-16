#ifndef WIRECELL_ISLICEFRAMESINK
#define WIRECELL_ISLICEFRAMESINK

#include "WireCellIface/ISliceFrame.h"
#include "WireCellIface/ISinkNode.h"

namespace WireCell {

    class ISliceFrameSink : public ISinkNode<ISliceFrame> {
       public:
        typedef std::shared_ptr<ISliceFrameSink> pointer;

        virtual ~ISliceFrameSink();

        virtual std::string signature() { return typeid(ISliceFrameSink).name(); }

        // supply:
        // virtual bool operator()(const IFrame::pointer& frame);
    };

}  // namespace WireCell

#endif
