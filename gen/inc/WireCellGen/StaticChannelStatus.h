/**
   The static channel status component provides a simple way to set
   channel status via configuration.  It does not provide for dynamic
   changes.
 */

#ifndef WIRECELLGEN_STATICCHANNELSTATUS
#define WIRECELLGEN_STATICCHANNELSTATUS

#include "WireCellIface/IChannelStatus.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"

#include <unordered_map>

namespace WireCell {
    namespace Gen {
        class StaticChannelStatus : public IChannelStatus, public IConfigurable {
           public:
            struct ChannelStatus {
                double gain;
                double shaping;
                ChannelStatus(double g = 14.0 * units::mV / units::fC, double s = 2.0 * units::us)
                  : gain(g)
                  , shaping(s)
                {
                }
            };
            typedef std::unordered_map<int, ChannelStatus> channel_status_map_t;

            StaticChannelStatus(double nominal_gain = 14.0 * units::mV / units::fC,
                                double nominal_shaping = 2.0 * units::us,
                                channel_status_map_t deviants = channel_status_map_t());
            virtual ~StaticChannelStatus();

            // IChannelStatus interface
            virtual double preamp_gain(int chid) const;
            virtual double preamp_shaping(int chid) const;

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            double m_nominal_gain, m_nominal_shaping;
            channel_status_map_t m_deviants;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
