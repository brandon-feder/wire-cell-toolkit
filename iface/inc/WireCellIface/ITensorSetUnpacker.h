#ifndef WIRECELL_IFACE_ITENSORSETUNPACKER
#define WIRECELL_IFACE_ITENSORSETUNPACKER

#include "WireCellIface/IFanoutNode.h"
#include "WireCellIface/ITensor.h"
#include "WireCellIface/ITensorSet.h"

#include <string>

namespace WireCell {
    /*
     * Unpack ITensorSet into tensors
     */
    class ITensorSetUnpacker : public IFanoutNode<ITensorSet, ITensor, 0> {
       public:
        virtual ~ITensorSetUnpacker();

        virtual std::string signature() { return typeid(ITensorSetUnpacker).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> output_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell

#endif