#ifndef WIRECELLIFACE_IWIRE
#define WIRECELLIFACE_IWIRE

#include "WireCellUtil/Point.h"
#include "WireCellIface/IData.h"
#include "WireCellIface/ISequence.h"
#include "WireCellIface/WirePlaneId.h"

#include <set>
#include <map>
#include <vector>
#include <memory>

namespace WireCell {

    /// Interface to information about a physical wire segment.
    class IWire : public IData<IWire> {
       public:
        virtual ~IWire();

        /// Detector-dependent, globally unique ID number.  Negative
        /// is illegal, not guaranteed consecutive.
        virtual int ident() const = 0;

        /// The ID of the plane this wire is in
        virtual WirePlaneId planeid() const = 0;

        /// Consecutive, zero-based index into an ordered sequence of
        /// wires in their plane
        virtual int index() const = 0;

        /// Detector-dependent electronics channel number, negative is
        /// illegal.  All wires with a common channel number are
        /// considered electrically connected.
        virtual int channel() const = 0;

        /// Return the number of wire segments between the channel
        /// input and this wire.  Wire directly attached to channel
        /// input is segment==0.
        virtual int segment() const = 0;

        /// Return the ray representing the wire segment.
        // fixme: may want to change this to a const reference to save the copy
        virtual WireCell::Ray ray() const = 0;

        /// Return the center point of the wire.  Convenience method.
        virtual WireCell::Point center() const;

    };  // class IWire

    bool ascending_index(IWire::pointer lhs, IWire::pointer rhs);

    /// Some common collections.
    typedef std::pair<IWire::pointer, IWire::pointer> IWirePair;

    // A set ordered by wire ident
    struct IWireCompareIdent {
        bool operator()(const IWire::pointer& lhs, const IWire::pointer& rhs) const
        {
            if (lhs->ident() == rhs->ident()) {
                return lhs.get() < rhs.get();  // break tie with pointer
            }
            return lhs->ident() < rhs->ident();
        }
    };
    typedef std::set<IWire::pointer, IWireCompareIdent> IWireSet;

    // Compare by wire index
    struct IWireCompareIndex {
        bool operator()(const IWire::pointer& lhs, const IWire::pointer& rhs) const
        {
            // fixme: should probably do something smarter here if two are not in same plane.....
            if (lhs->index() == rhs->index()) {
                return lhs.get() < rhs.get();  // break tie with pointer
            }
            return lhs->index() < rhs->index();
        }
    };
    typedef std::set<IWire::pointer, IWireCompareIndex> IWireIndexSet;

    // A set ordered by wire segment
    struct IWireCompareSegment {
        bool operator()(const IWire::pointer& lhs, const IWire::pointer& rhs) const
        {
            if (lhs->segment() == rhs->segment()) {
                return lhs.get() < rhs.get();  // break tie with pointer
            }
            return lhs->segment() < rhs->segment();
        }
    };
    typedef std::set<IWire::pointer, IWireCompareSegment> IWireSegmentSet;

}  // namespace WireCell

#endif
