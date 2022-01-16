/** This component provides identical channel responses for all
 * channels and which use the nominal electronics response
 * parameterized by gain and shaping time. */

#ifndef WIRECELLSIGPROC_NOMINALCHANNELRESPONSE
#define WIRECELLSIGPROC_NOMINALCHANNELRESPONSE

#include "WireCellIface/IChannelResponse.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"

namespace WireCell {
    namespace SigProc {
        class NominalChannelResponse : public IChannelResponse, public IConfigurable {
           public:
            NominalChannelResponse(double gain = 14 * WireCell::units::mV / WireCell::units::fC,
                                   double shaping = 2 * WireCell::units::us,
                                   const Binning& binning = Binning(100, 0, 10 * WireCell::units::us));

            virtual ~NominalChannelResponse();

            // IChannelResponse
            virtual const Waveform::realseq_t& channel_response(int channel_ident) const;
            virtual Binning channel_response_binning() const;

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            double m_gain, m_shaping;
            Binning m_bins;

            Waveform::realseq_t m_cr;  // cache
        };

    }  // namespace SigProc

}  // namespace WireCell
#endif
