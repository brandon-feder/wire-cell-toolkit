/* A cluster filter is a function node which takes one cluster and produces another.
 */

#ifndef WIRECELL_ICLUSTERFILTER
#define WIRECELL_ICLUSTERFILTER

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/ICluster.h"

namespace WireCell {

    class IClusterFilter : public IFunctionNode<ICluster, ICluster> {
       public:
        typedef std::shared_ptr<IClusterFilter> pointer;

        virtual ~IClusterFilter();

        virtual std::string signature() { return typeid(IClusterFilter).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
