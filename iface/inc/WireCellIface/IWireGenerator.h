#ifndef WIRECELLIFACE_IWIREGENERATOR
#define WIRECELLIFACE_IWIREGENERATOR

#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IWireParameters.h"
#include "WireCellIface/IWire.h"

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    /** A wire generator is a function node takes a set of parameters
     * and generates a vector of wires.
     */
    class IWireGenerator : public IFunctionNode<IWireParameters, IWire::vector> {
       public:
        virtual ~IWireGenerator();

        virtual std::string signature() { return typeid(IWireGenerator).name(); }
    };

}  // namespace WireCell

#endif
