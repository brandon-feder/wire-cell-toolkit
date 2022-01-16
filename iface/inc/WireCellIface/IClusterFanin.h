#ifndef WIRECELL_IFACE_ICLUSTERFANIN
#define WIRECELL_IFACE_ICLUSTERFANIN

#include "WireCellIface/IFaninNode.h"
#include "WireCellIface/ICluster.h"

namespace WireCell {
    /** A cluster fan-in component takes N clusters on input ports and
     * produces a single output cluster.  The merge policy and
     * restriction on the number of inputs, if any, are left to the
     * implementation.
     */
    class IClusterFanin : public IFaninNode<ICluster, ICluster, 0> {
       public:
        virtual ~IClusterFanin();

        virtual std::string signature() { return typeid(IClusterFanin).name(); }

        // Subclass must implement:
        virtual std::vector<std::string> input_types() = 0;
        // and the already abstract:
        // virtual bool operator()(const input_vector& invec, output_pointer& out) = 0;
    };
}  // namespace WireCell

#endif
