#ifndef WIRECELL_IDUCTOR
#define WIRECELL_IDUCTOR

#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    /** A ductor consumes depositions and collects the response on
     * electrodes due to the depos drifting past as a number of
     * sampled waveforms.  This interface does not specify the units
     * of the samples and they are left up to the implementation. But,
     * they are typically either current or charge if only electric
     * field responses are applied or voltage if additionally an
     * electronics response model is applied.
     *
     * Implementation may also apply digitization in which case the
     * floating point samples should be considered integer.
     * Digitization may also be implemented as an IFrameFilter.
     */
    class IDuctor : public IQueuedoutNode<IDepo, IFrame> {
       public:
        typedef std::shared_ptr<IDuctor> pointer;

        virtual ~IDuctor();

        virtual std::string signature() { return typeid(IDuctor).name(); }
    };
}  // namespace WireCell

#endif
