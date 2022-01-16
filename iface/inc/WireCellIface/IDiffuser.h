#ifndef WIRECELL_IDIFFUSER
#define WIRECELL_IDIFFUSER

#include "WireCellIface/IQueuedoutNode.h"

#include "WireCellIface/IDepo.h"
#include "WireCellIface/IDiffusion.h"

namespace WireCell {

    /** Interface for a diffuser.
     *
     * This buffer node takes IDepo deposition objects and returns a
     * queue of IDiffusion diffusion objects.  Deposition objects must
     * be added in strict time (but not space) order.  Only those
     * collected depositions which are causally disconnected from the
     * newest deposition will be processed.  See WireCell::Diffuser as
     * one example implementation.
     */
    class IDiffuser : public IQueuedoutNode<IDepo, IDiffusion> {
       public:
        virtual ~IDiffuser();

        virtual std::string signature() { return typeid(IDiffuser).name(); }
    };

}  // namespace WireCell

#endif
