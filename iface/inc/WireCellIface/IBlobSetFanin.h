#ifndef WIRECELL_IFACE_IBLOBSETFANIN
#define WIRECELL_IFACE_IBLOBSETFANIN

#include "WireCellIface/IFaninNode.h"
#include "WireCellIface/IBlobSet.h"

namespace WireCell {
    /** A blob set fan-in component takes blobs sets on N input ports
     * and produces a vector of them on its output port.
     */
    class IBlobSetFanin : public IFaninNode<IBlobSet, IBlobSet, 0> {
       public:
        virtual ~IBlobSetFanin();

        virtual std::string signature() { return typeid(IBlobSetFanin).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> input_types() = 0;
        // as well as the already abstract:
        // virtual bool operator()(const input_vector& invec, output_pointer& out) = 0;
    };
}  // namespace WireCell

#endif
