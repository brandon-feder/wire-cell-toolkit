#ifndef WIRECELL_IDEPOMERGER
#define WIRECELL_IDEPOMERGER

#include "WireCellIface/IHydraNode.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    // FIXME: this is wrong with the shared_ptr.
    // see comments in IHydraNode.  Needs fixes in TupleHelper.
    class IDepoMerger : public IHydraNode<std::tuple<IDepo, IDepo>, std::tuple<IDepo> > {
       public:
        typedef std::shared_ptr<IDepoMerger> pointer;
        virtual ~IDepoMerger();

        virtual std::string signature() { return typeid(IDepoMerger).name(); }

        // subclass supply:
        // virtual bool operator()(input_queues_type& inqs,
        //                         output_queues_type& outqs);
    };

}  // namespace WireCell
#endif
