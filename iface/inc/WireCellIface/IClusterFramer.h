/* A cluster framer produces a frame from a cluster.
 */

#ifndef WIRECELL_ICLUSTERFRAMER
#define WIRECELL_ICLUSTERFRAMER

#include "WireCellIface/ICluster.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/IFunctionNode.h"

namespace WireCell {
    class IClusterFramer : public IFunctionNode<ICluster, IFrame> {
       public:
        virtual ~IClusterFramer();

        virtual std::string signature() { return typeid(IClusterFramer).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
