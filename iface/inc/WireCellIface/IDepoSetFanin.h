#ifndef WIRECELL_IFACE_IDEPOSETFANIN
#define WIRECELL_IFACE_IDEPOSETFANIN

#include "WireCellIface/IFaninNode.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {
    /** A depo set fan-in component takes depo sets on N input ports
     * and produces a vector of them on its output port.
     */
    class IDepoSetFanin : public IFaninNode<IDepoSet, IDepoSet, 0> {
       public:
        virtual ~IDepoSetFanin();

        virtual std::string signature() { return typeid(IDepoSetFanin).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> input_types() = 0;
        // as well as the already abstract:
        // virtual bool operator()(const input_vector& invec, output_pointer& out) = 0;
    };
}  // namespace WireCell

#endif
