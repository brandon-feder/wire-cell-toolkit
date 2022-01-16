/** Vary field response to study systematics.
 */

#ifndef WIRECELL_GEN_RESPONSESYS
#define WIRECELL_GEN_RESPONSESYS

#include "WireCellIface/IWaveform.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Response.h"

namespace WireCell {
    namespace Gen {

        class ResponseSys : public IWaveform, public IConfigurable {
           public:
            ResponseSys(int nticks = 10000, double start = 0.0 * units::us, double tick = 0.5 * units::us,
                        double magnitude = 1.0, double time_smear = 0.0 * units::us, double offset = 0.0 * units::us);

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            // The starting point of the sampling
            virtual double waveform_start() const;
            // The sampling period aka bin width
            virtual double waveform_period() const;
            // The collection of samples
            virtual const sequence_type& waveform_samples() const;
            // The collection of samples rebinned
            virtual sequence_type waveform_samples(const WireCell::Binning& tbins) const;

           private:
            Response::SysResp* m_sysresp;
            Configuration m_cfg;
            sequence_type m_wave;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
