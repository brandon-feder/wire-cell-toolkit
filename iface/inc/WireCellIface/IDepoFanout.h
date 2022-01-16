#ifndef WIRECELL_IFACE_IDEPOFANOUT
#define WIRECELL_IFACE_IDEPOFANOUT

#include "WireCellIface/IFanoutNode.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    // This fixes the type for input and output slots.  The
    // multiplicity MUST be set by the subclass and the method
    // output_types() must be reimplemented
    class IDepoFanout : public IFanoutNode<IDepo, IDepo, 0> {
       public:
        virtual ~IDepoFanout();

        virtual std::string signature() { return typeid(IDepoFanout).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> output_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell
#endif
