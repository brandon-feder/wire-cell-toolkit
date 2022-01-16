/**

A slice associates an IChannel with a value representing activity over
some time span.

The time span of the slice is given in the WCT system of units.
Typically it is some multiple of sampling ticks, but need not be.

The slice also caries an "ident" which definition is
application-dependent.

*/

#ifndef WIRECELL_ISLICE
#define WIRECELL_ISLICE

#include "WireCellIface/IData.h"
#include "WireCellIface/IFrame.h"

#include <unordered_map>

namespace WireCell {

    class ISlice : public IData<ISlice> {
       public:
        virtual ~ISlice();

        // The type for the value of the per channel metric (aka charge)
        typedef float value_t;

        // A sample is a channels value in the time slice.
        typedef std::pair<IChannel::pointer, value_t> pair_t;
        typedef std::unordered_map<IChannel::pointer, value_t> map_t;

        // Pointer back to IFrame from which this ISlice was created.
        virtual IFrame::pointer frame() const = 0;

        // Return a opaque numerical identifier of this time slice
        // unique in some broader context.
        virtual int ident() const = 0;

        // Return the start time of this time slice relative to frame's time
        virtual double start() const = 0;

        // Return the time span of this slice
        virtual double span() const = 0;

        // The activity in the form of a channel/value map;
        virtual map_t activity() const = 0;
    };
}  // namespace WireCell

#endif
