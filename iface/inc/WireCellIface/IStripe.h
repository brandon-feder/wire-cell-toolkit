#ifndef WIRECELL_ISTRIPE
#define WIRECELL_ISTRIPE

#include "WireCellIface/IData.h"
#include "WireCellIface/IChannel.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Units.h"

#include <vector>

namespace WireCell {

    /** An interface to information about a "stripe" of channels.
     *
     * A stripe is an ordered and "contiguous" set of channels.
     *
     * Order and conent of the set of channels is application
     * dependent.  Typically, and why this interface is named as
     * "stripe", the set will consist of the channels that are
     * attached to wires (and as always it means wire segments) that
     * when taken together form a contiguous strip on the faces of the
     * anode plane.  For wrapped wire planes the strip is like that of
     * a flattened helix.  For unwrapped wire planes one might better
     * call this a "strip" (and indeed "strip" used in the context of
     * IBlob to refer to a contiguous region on a single face).
     */
    class IStripe : public IData<IStripe> {
       public:
        virtual ~IStripe();

        typedef float value_t;

        /// A sample is a channel's value in the time slice.
        typedef std::pair<IChannel::pointer, value_t> pair_t;
        typedef std::vector<pair_t> vector_t;

        /// An identifier for this stripe.
        virtual int ident() const = 0;

        /// the contiguous, ordered, per channel values.
        virtual vector_t values() const = 0;
    };

}  // namespace WireCell

#endif
