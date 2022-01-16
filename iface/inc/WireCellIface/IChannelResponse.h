/** A channel response gives access to a per-channel response
 * function.  This is typically used to return the electronics
 * response for a channel.  Depending on implementation this may be an
 * ideal response shared by all or it could be some measured or
 * calibration response that differs for each channel. */

#ifndef WIRECELLIFACE_ICHANNELRESPONSE
#define WIRECELLIFACE_ICHANNELRESPONSE

#include "WireCellUtil/IComponent.h"

#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Waveform.h"

namespace WireCell {

    class IChannelResponse : public IComponent<IChannelResponse> {
       public:
        virtual ~IChannelResponse();

        /// Provide the channel response for the given channel ID
        /// number.  Note the binning of the returned waveform should
        /// be coordinated through configuration.
        virtual const Waveform::realseq_t& channel_response(int channel_ident) const = 0;

        /// Return the binning that the channel_response follows.
        virtual Binning channel_response_binning() const = 0;
    };
}  // namespace WireCell
#endif
