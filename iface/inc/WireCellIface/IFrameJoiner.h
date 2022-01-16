#ifndef WIRECELL_IFRAMEJOINER
#define WIRECELL_IFRAMEJOINER

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IJoinNode.h"
#include "WireCellIface/IFrame.h"

namespace WireCell {

    /** A frame joiner is something that takes in two frames and sends
     *  out one.
     */
    class IFrameJoiner : public IJoinNode<std::tuple<IFrame, IFrame>, IFrame> {
       public:
        typedef std::shared_ptr<IFrameJoiner> pointer;

        virtual ~IFrameJoiner();

        virtual std::string signature() { return typeid(IFrameJoiner).name(); }

        // subclass supply:
        // virtual bool operator()(const input_tuple_type& intup,
        //                         output_pointer& out);
    };

}  // namespace WireCell

#endif
