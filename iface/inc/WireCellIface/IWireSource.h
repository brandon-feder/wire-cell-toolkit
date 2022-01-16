#ifndef WIRECELLIFACE_IWIRESOURCE
#define WIRECELLIFACE_IWIRESOURCE

#include "WireCellIface/ISourceNode.h"
#include "WireCellIface/IWire.h"

namespace WireCell {

    /** Return a vector of wires.
     */
    class IWireSource : public ISourceNode<IWire::vector> {
       public:
        virtual ~IWireSource();

        virtual std::string signature() { return typeid(IWireSource).name(); }
        // supply:
        // virtual bool operator()(output_pointer& wires);
    };

}  // namespace WireCell

#endif
