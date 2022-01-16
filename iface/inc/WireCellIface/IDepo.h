#ifndef WIRECELL_IDEPO
#define WIRECELL_IDEPO

#include "WireCellIface/IData.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Units.h"

#include <set>

namespace WireCell {

    /** An interface to information about a deposition of charge.
     */
    class IDepo : public IData<IDepo> {
       public:
        virtual ~IDepo();

        /// The location of the deposition.
        virtual const Point& pos() const = 0;

        /// The number of seconds from some absolute start time to
        /// when the deposition occur ed.
        virtual double time() const = 0;

        /// The number charge (in units of number of electrons) deposited.
        virtual double charge() const = 0;

        /// The energy (in units of MeV) deposited.
        virtual double energy() const = 0;

        /// Track ID from Geant4
        virtual int id() const = 0;

        /// PDG code from Geant4
        virtual int pdg() const = 0;

        /// If the deposition is drifted, this may allow access to the original.
        virtual pointer prior() const = 0;

        /// Any (half width) extent in the longitudinal (drift)
        /// direction (distance).  The distribution is implicit but
        /// typically it is taken that this is a Gaussian sigma.
        virtual double extent_long() const { return 0.0; }

        /// Any (half width) extent in the transverse (pitch)
        /// direction (distance).  The distribution is implicit but
        /// typically it is taken that this is a Gaussian sigma.
        virtual double extent_tran() const { return 0.0; }
    };

    /// Simple utility to return a vector of depositions formed by
    /// walking the prior() chain.  The vector begins with the most
    /// recent.
    IDepo::vector depo_chain(IDepo::pointer recent);

    /// Compare how "far" two depositions are from the origin along
    /// the drift-line (metric: dT + dX/V_drift) given a drift
    /// velocity.  Note: if drifting is toward a "back" face of an
    /// anode then the drift speed should be negative in order to
    /// indicate drift is in the postitive X direction..
    struct IDepoDriftCompare {
        double drift_speed;
        IDepoDriftCompare(double drift_speed = 1.6 * units::mm / units::microsecond)
          : drift_speed(drift_speed){};
        bool operator()(const IDepo::pointer& lhs, const IDepo::pointer& rhs) const
        {
            double t1 = lhs->time() + lhs->pos().x() / drift_speed;
            double t2 = rhs->time() + rhs->pos().x() / drift_speed;
            if (t1 == t2) {
                // make sure there are no ties due to precision!
                return lhs.get() < rhs.get();
            }
            return t1 < t2;
        }
    };
    typedef std::set<IDepo::pointer, IDepoDriftCompare> DepoTauSortedSet;

    /// Compare two IDepo::pointer by time (ascending).  x is used to break tie
    bool ascending_time(const WireCell::IDepo::pointer& lhs, const WireCell::IDepo::pointer& rhs);

    /// Compare two IDepo::pointers for by time, descending.   x is used to break tie
    bool descending_time(const WireCell::IDepo::pointer& lhs, const WireCell::IDepo::pointer& rhs);

}  // namespace WireCell

#endif
