#ifndef WIRECELLGEN_DEPOPLANEX
#define WIRECELLGEN_DEPOPLANEX

#include "WireCellUtil/Point.h"
#include "WireCellUtil/Units.h"
#include "WireCellIface/IDepo.h"

#include <deque>

namespace WireCell {
    namespace Gen {

        /** A DepoPlaneX collects depositions and drifts them to the
         * given plane assuming a uniform drift velocity which is in
         * the negative X direction.
         *
         * It is assumed new depositions are added strictly in time
         * order.  They will then be drifted and maintained in the
         * order of their time at the plane.
         *
         * The time of the most recently added depo sets a high water
         * mark in time at the plane such that all newly added depos
         * must (causally) come later.  Any depos older than this time
         * are considered "frozen out" as nothing can change their
         * ordering.
         */

        class DepoPlaneX {
           public:
            typedef std::deque<IDepo::pointer> frozen_queue_t;
            typedef DepoTauSortedSet working_queue_t;

            DepoPlaneX(double planex = 0.0 * units::cm, double speed = 1.6 * units::millimeter / units::microsecond);

            /// Add a deposition and drift it into the queue at the
            /// plane.  Depos must be strictly added in (local) time
            /// order.  The drifted depo is returned (and held).
            IDepo::pointer add(const IDepo::pointer& depo);

            /// The time a deposition would have if it drifts to the plane
            double proper_time(IDepo::pointer depo) const;

            /// Return the time at the plane before which the order of
            /// collected depositions are guaranteed (causally) to
            /// remain unchanged as any new depos are added.
            double freezeout_time() const;

            /// Force any remaining "thawed" depos in the queue to be frozen out.
            void freezeout();

            /// Return ordered vector of all depositions at the plane
            /// with times not later than the given time.  The given
            /// time is typically the freezeout time.  If a time later
            /// than the freezout time is given it may cause depos to
            /// be artificially frozen out.
            IDepo::vector pop(double time);

            // Access internal const data structures to assist in debugging
            const frozen_queue_t& frozen_queue() const { return m_frozen; }
            const working_queue_t& working_queue() const { return m_queue; }

           private:
            double m_planex, m_speed;
            working_queue_t m_queue;
            frozen_queue_t m_frozen;

            /// Move all froze-out depos to the frozen queue
            void drain(double time);
        };

    }  // namespace Gen

}  // namespace WireCell

#endif
