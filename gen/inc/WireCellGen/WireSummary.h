#ifndef WIRECELL_WIRESUMMARY
#define WIRECELL_WIRESUMMARY

#include "WireCellIface/IWireSummary.h"

namespace WireCell {

    /** Default WireSummary which is also a wire sink and a wire sequence.
     */
    class WireSummary : public IWireSummary {
       public:
        WireSummary(const IWire::vector& wires);
        virtual ~WireSummary();

        /// Return the bounding box of the wire planes.
        virtual const BoundingBox& box() const;

        /// Return the closest wire along the pitch direction to the
        /// given point in the given wire plane.  It is assumed the
        /// point is in the (Y-Z) bounding box of the wire plane.
        virtual IWire::pointer closest(const Point& point, WirePlaneId wpid) const;

        /// Return a pair of adjacent wires from the given plane which
        /// bound the given point along the pitch direction.  The pair
        /// is ordered by increasing wire index number.  If one or
        /// both sides of the point are unbound by wire (segments) the
        /// associated pointer will be zero.  It is assumed the point
        /// is in the (Y-Z) bounding box of the wire plane.
        virtual IWirePair bounding_wires(const Point& point, WirePlaneId wpid) const;

        /// Return the distance along the pitch of the given wire
        /// plane to the given point as measured from the zeroth wire.
        virtual double pitch_distance(const Point& point, WirePlaneId wpid) const;

        /// Return a unit vector along the direction of the pitch.
        virtual const Vector& pitch_direction(WirePlaneId wpid) const;

        virtual IWire::vector by_channel(int channel) const;

       private:
        struct WireSummaryCache;
        WireSummaryCache* m_cache;
    };
}  // namespace WireCell

#endif
