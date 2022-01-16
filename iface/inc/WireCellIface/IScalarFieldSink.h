#ifndef WIRECELL_ISCALARFIELDSINK
#define WIRECELL_ISCALARFIELDSINK

#include "WireCellIface/ISinkNode.h"
#include "WireCellUtil/Point.h"

namespace WireCell {

    /** Base class for a sink of cells. */
    class IScalarFieldSink : public ISinkNode<ScalarField> {
       public:
        virtual ~IScalarFieldSink();

        virtual std::string signature() { return typeid(IScalarFieldSink).name(); }
        /// supply:
        /// virtual bool operator()(const std::shared_ptr<ScalarField>& field);
    };

}  // namespace WireCell

#endif
