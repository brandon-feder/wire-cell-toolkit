#ifndef WIRECELL_TRACKDEPOS
#define WIRECELL_TRACKDEPOS

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"
#include "WireCellAux/Logger.h"

#include <tuple>
#include <deque>

namespace WireCell {

    namespace Gen {

        /// A producer of depositions created from some number of simple, linear tracks.
        class TrackDepos : public Aux::Logger,
                           public IDepoSource, public IConfigurable {
           public:
            /// Create tracks with depositions every stepsize and assumed
            /// to be traveling at clight.
            TrackDepos(double stepsize = 1.0 * units::millimeter, double clight = 1.0);
            virtual ~TrackDepos();

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            /// Add track starting at given <time> and stretching across given
            /// ray.  The <dedx> gives a uniform charge/distance and if < 0
            /// then it gives the (negative of) absolute amount of charge per
            /// deposition.
            void add_track(double time, const WireCell::Ray& ray, double dedx = -1.0);

            /// ISourceNode
            virtual bool operator()(IDepo::pointer& out);

            WireCell::IDepo::vector depos();

            typedef std::tuple<double, Ray, double> track_t;
            std::vector<track_t> tracks() const { return m_tracks; }

           private:
            double m_stepsize;
            double m_clight;
            std::deque<WireCell::IDepo::pointer> m_depos;
            std::vector<track_t> m_tracks;  // collect for posterity
            int m_count;
        };

    }  // namespace Gen
}  // namespace WireCell

#endif
