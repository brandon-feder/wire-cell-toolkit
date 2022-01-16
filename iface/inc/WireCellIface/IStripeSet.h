#ifndef WIRECELL_ISTRIPESET
#define WIRECELL_ISTRIPESET

#include "WireCellIface/IStripe.h"

namespace WireCell {

    /** An interface to information about a collection of IStripe.
     */
    class IStripeSet : public IData<IStripeSet> {
       public:
        virtual ~IStripeSet();

        /// Return some identifier number that is unique to this set.
        virtual int ident() const = 0;

        /// Return the stripes in this set.  There is no ordering requirement.
        virtual IStripe::vector stripes() const = 0;
    };
}  // namespace WireCell

#endif
