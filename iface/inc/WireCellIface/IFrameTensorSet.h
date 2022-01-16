#ifndef WIRECELL_IFRAMETENSORSET
#define WIRECELL_IFRAMETENSORSET

#include "WireCellIface/IFrame.h"
#include "WireCellIface/ITensorSet.h"
#include "WireCellIface/IFunctionNode.h"

namespace WireCell {

    /*! An IFrame to ITensorSet converter interface. */
    class IFrameTensorSet : public IFunctionNode<IFrame, ITensorSet> {
       public:
        typedef std::shared_ptr<IFrameTensorSet> pointer;

        virtual ~IFrameTensorSet(){};

        virtual std::string signature() { return typeid(IFrameTensorSet).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
