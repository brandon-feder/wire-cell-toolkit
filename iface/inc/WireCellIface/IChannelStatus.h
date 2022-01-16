/**
   IChannelStatus provides information about each channel which
   potentially could change during the run.
 */

#ifndef WIRECELLIFACE_ICHANNELSTATUS
#define WIRECELLIFACE_ICHANNELSTATUS

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class IChannelStatus : public IComponent<IChannelStatus> {
       public:
        virtual ~IChannelStatus();

        /// Return the current gain for the preamplifier in units of
        /// [voltage]/[charge].
        virtual double preamp_gain(int chid) const = 0;

        /// Return the current shaping time of the preamplifier for
        /// the given channel in units of [time].
        virtual double preamp_shaping(int chid) const = 0;
    };
}  // namespace WireCell

#endif
