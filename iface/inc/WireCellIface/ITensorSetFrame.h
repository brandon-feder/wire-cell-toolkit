#ifndef WIRECELL_ITENSORSETFRAME
#define WIRECELL_ITENSORSETFRAME

#include "WireCellIface/IFrame.h"
#include "WireCellIface/ITensorSet.h"
#include "WireCellIface/IFunctionNode.h"

namespace WireCell {

    /*! Interface which converts from a set of tensors to a frame. */
    class ITensorSetFrame : public IFunctionNode<ITensorSet, IFrame> {
       public:
        typedef std::shared_ptr<ITensorSetFrame> pointer;

        virtual ~ITensorSetFrame() {}

        virtual std::string signature() { return typeid(ITensorSetFrame).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };
}  // namespace WireCell

#endif
