#ifndef WIRECELL_IFACE_IDEPOSETFANOUT
#define WIRECELL_IFACE_IDEPOSETFANOUT

#include "WireCellIface/IFanoutNode.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    // This fixes the type for input and output slots.  The
    // multiplicity MUST be set by the subclass and the method
    // output_types() must be reimplemented
    class IDepoSetFanout : public IFanoutNode<IDepoSet, IDepoSet, 0> {
       public:
        virtual ~IDepoSetFanout();

        virtual std::string signature() { return typeid(IDepoSetFanout).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> output_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell
#endif
