#ifndef WIRECELL_IWIRESUMMARY
#define WIRECELL_IWIRESUMMARY

#include "WireCellUtil/BoundingBox.h"
#include "WireCellIface/IWire.h"

namespace WireCell {

    /** Interface to summary information about wires.
     *
     * Note: the actual implementation of these are likely just a
     * bunch of stand-alone functions.  They are couched into a class
     * in order to allow for caching optimization and to avoid having
     * to constantly pass in the wires.
     */
    class IWireSummary : public IData<IWireSummary> {
       public:
        // typedef std::shared_ptr<IWireSummary> pointer;

        virtual ~IWireSummary();

        /// Return the bounding box of the wire planes.
        virtual const BoundingBox& box() const = 0;

        /// Return the closest wire along the pitch direction to the
        /// given point in the given wire plane.  It is assumed the
        /// point is in the (Y-Z) bounding box of the wire plane.
        virtual IWire::pointer closest(const Point& point, WirePlaneId wpid) const = 0;

        /// Return a pair of adjacent wires from the given plane which
        /// bound the given point along the pitch direction.  The pair
        /// is ordered by increasing wire index number.  If one or
        /// both sides of the point are unbound by wire (segments) the
        /// associated pointer will be zero.  It is assumed the point
        /// is in the (Y-Z) bounding box of the wire plane.
        virtual IWirePair bounding_wires(const Point& point, WirePlaneId wpid) const = 0;

        /// Return the distance along the pitch of the given wire
        /// plane to the given point as measured from the zeroth wire.
        virtual double pitch_distance(const Point& point, WirePlaneId wpid) const = 0;

        /// Return a unit vector along the direction of the pitch.
        virtual const Vector& pitch_direction(WirePlaneId wpid) const = 0;

        /// Return all wires, in order of segment number, attached to the channel.
        virtual IWire::vector by_channel(int channel) const = 0;
    };
}  // namespace WireCell

#endif
