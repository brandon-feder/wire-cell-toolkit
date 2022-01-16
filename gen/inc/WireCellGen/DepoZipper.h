/** Make a frame from depos using an ImpactZipper.

    See also the very similar DepoTransform which is newer and faster.
 */

#ifndef WIRECELLGEN_DEPOZIPPER
#define WIRECELLGEN_DEPOZIPPER

#include "WireCellIface/IDepoFramer.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IPlaneImpactResponse.h"
#include "WireCellIface/IAnodePlane.h"

namespace WireCell {
    namespace Gen {

        class DepoZipper : public IDepoFramer, public IConfigurable {
           public:
            DepoZipper();
            virtual ~DepoZipper();

            virtual bool operator()(const input_pointer& in, output_pointer& out);

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            IAnodePlane::pointer m_anode;
            IRandom::pointer m_rng;
            std::vector<IPlaneImpactResponse::pointer> m_pirs;

            double m_start_time;
            double m_readout_time;
            double m_tick;
            double m_drift_speed;
            double m_nsigma;
            int m_frame_count;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
