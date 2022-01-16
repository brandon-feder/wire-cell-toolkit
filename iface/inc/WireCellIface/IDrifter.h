#ifndef WIRECELL_IDRIFTER
#define WIRECELL_IDRIFTER

#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    /** A drifter takes in depositions at one location and drifts them
     * to another location subject to a drift velocity.  Drifters may
     * assume their input is time ordered and must assure the output
     * remains time-ordered (regardless of what time/space mixing may
     * occur).  In general, it means the drifter must buffer some
     * number of depositions to assure it's seen "enough" to determine
     * that its earliest drifted deposition will not superseded by any
     * subsequent ones.
     */
    class IDrifter : public IQueuedoutNode<IDepo, IDepo> {
       public:
        typedef std::shared_ptr<IDrifter> pointer;

        virtual ~IDrifter();

        virtual std::string signature() { return typeid(IDrifter).name(); }

        /// supply:
        // virtual bool operator()(const input_pointer& in, output_queue& outq) = 0;
    };
}  // namespace WireCell

#endif
