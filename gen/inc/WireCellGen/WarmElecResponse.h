/** An ElecResponse waveform is that due to sending a unit charge
 * pulse into an amplifier with gain and shaping and a particular
 * response function of the ICARUS warm electronics.  */

#ifndef WIRECELL_GEN_WARMELECRESPONSE
#define WIRECELL_GEN_WARMELECRESPONSE

#include "WireCellIface/IWaveform.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Response.h"

namespace WireCell {
    namespace Gen {

        class WarmElecResponse : public IWaveform, public IConfigurable {
           public:
            WarmElecResponse(int nticks = 10000, double t0 = 0, double gain = 30.0 * units::mV / units::fC,
                             double shaping = 1.3 * units::us, double postgain = 1.0, double tick = 0.4 * units::us);
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
            Response::WarmElec* m_warmresp;
            Configuration m_cfg;
            sequence_type m_wave;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
