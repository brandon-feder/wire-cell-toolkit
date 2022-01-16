#ifndef WIRECELL_IPROCESSOR
#define WIRECELL_IPROCESSOR

#include "WireCellUtil/Interface.h"

namespace WireCell {

    /** Base interface for any DFP graph vertex.
     */
    class IProcessor : virtual public Interface {
       public:
        virtual ~IProcessor();

        /// Called before any outside data is fed to the DFP graph.
        // virtual void initialize() { }

        /// Called after the DFP graph execution has finished.
        virtual void finalize() {}

        /// Called any time the DFP graph will be restarted.
        // virtual void reset() { }
    };

}  // namespace WireCell

#endif
