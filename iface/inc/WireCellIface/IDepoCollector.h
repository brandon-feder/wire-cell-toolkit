#ifndef WIRECELL_IDEPOCOLLECTOR
#define WIRECELL_IDEPOCOLLECTOR

#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IDepo.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    /** A depo collector if fed depos one at a time, possibly applying
     * some filtering, until some condition is met at which time it
     * produces a DepoSet.
     *
     * No depos will be retained after an EOS is received and it will
     * lead to an EOS being emitted.
     */
    class IDepoCollector : public IQueuedoutNode<IDepo, IDepoSet> {
       public:
        typedef std::shared_ptr<IDepoCollector> pointer;

        virtual ~IDepoCollector();

        virtual std::string signature() { return typeid(IDepoCollector).name(); }

        // implement:
        // virtual bool operator()(const input_pointer& depo, output_queue& deposetqueue);
    };
}  // namespace WireCell

#endif
