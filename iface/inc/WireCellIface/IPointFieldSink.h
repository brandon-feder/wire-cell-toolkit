#ifndef WIRECELL_IPOINTFIELDSINK
#define WIRECELL_IPOINTFIELDSINK

#include "WireCellIface/ISinkNode.h"
#include "WireCellUtil/Point.h"

namespace WireCell {

    /** Base class for a sink of cells. */
    class IPointFieldSink : public ISinkNode<PointVector> {
       public:
        virtual ~IPointFieldSink();

        virtual std::string signature() { return typeid(IPointFieldSink).name(); }
        /// supply:
        /// virtual bool operator()(const std::shared_ptr<PointVector>& field);
    };

}  // namespace WireCell

#endif
