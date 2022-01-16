/* Collect slices.
 */

#ifndef WIRECELL_ISLICEFRAME
#define WIRECELL_ISLICEFRAME

#include "WireCellIface/ISlice.h"

namespace WireCell {

    /** An interface to collection of slices.
     */
    class ISliceFrame : public IData<ISliceFrame> {
       public:
        virtual ~ISliceFrame();

        /// Return some identifier number that is unique to this slice frame.
        virtual int ident() const = 0;

        /// A reference time.
        virtual double time() const = 0;

        /// Return the slices.
        virtual ISlice::vector slices() const = 0;
    };
}  // namespace WireCell

#endif
