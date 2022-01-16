/**

    A blob is a region in the 2D plane transverse to the drift
    direction.  It is made up of a number of logically coplanar
    "layers".  Each layer is bound by a pair of parallel rays.  Rays
    may correspond to wires or to overall bounds of senitivity of the
    anode plane face.  See manual for details.


    A blob is assumed to exist in some context which provides its
    location along a drift direction aka time and a logical assocation
    with a particular anode face.  See IBlobSet for example.
 */

#ifndef WIRECELL_IBLOB
#define WIRECELL_IBLOB

#include "WireCellIface/IAnodeFace.h"
#include "WireCellIface/ISlice.h"

#include "WireCellUtil/RayTiling.h"

namespace WireCell {

    class IBlob : public IData<IBlob> {
       public:
        virtual ~IBlob();

        // An identifier for this blob.
        virtual int ident() const = 0;

        // An associated value, aka "charge", may be 0.0.
        virtual float value() const = 0;

        // And its uncertainty.  A "weight" is considered 1.0/uncertainty,
        virtual float uncertainty() const = 0;

        // Return the anode plane face the blob was made on.
        virtual IAnodeFace::pointer face() const = 0;

        // Return the time slice that the blob was made from.
        virtual ISlice::pointer slice() const = 0;

        // Return the blob data describing the shape in terms of
        // wire-in-plane indices
        virtual const RayGrid::Blob& shape() const = 0;
    };

}  // namespace WireCell

#endif
