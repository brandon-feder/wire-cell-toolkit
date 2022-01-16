#ifndef WIRECELLIFACE_IFRAMESPLITTER
#define WIRECELLIFACE_IFRAMESPLITTER

#include "WireCellIface/IFrame.h"
#include "WireCellIface/ISplitNode.h"

namespace WireCell {

    class IFrameSplitter : public ISplitNode<IFrame, std::tuple<IFrame, IFrame> > {
       public:
        typedef std::shared_ptr<IFrameSplitter> pointer;
        virtual ~IFrameSplitter();

        virtual std::string signature() { return typeid(IFrameSplitter).name(); }

        // subclass supply:
        // 	virtual bool operator()(const input_pointer& in, output_tuple_type& out) = 0;
    };

}  // namespace WireCell
#endif
