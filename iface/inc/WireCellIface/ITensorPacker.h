#ifndef WIRECELL_IFACE_ITENSORPACKER
#define WIRECELL_IFACE_ITENSORPACKER

#include "WireCellIface/IFaninNode.h"
#include "WireCellIface/ITensor.h"
#include "WireCellIface/ITensorSet.h"

#include <string>

namespace WireCell {
    /*
     * Pack ITensors into ITensorSet
     */
    class ITensorPacker : public IFaninNode<ITensor, ITensorSet, 0> {
       public:
        virtual ~ITensorPacker();

        virtual std::string signature() { return typeid(ITensorPacker).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> input_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_pointer& in, output_vector& outv);
    };
}  // namespace WireCell

#endif