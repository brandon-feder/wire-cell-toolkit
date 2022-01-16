#ifndef WIRECELL_ICHANNELFILTER
#define WIRECELL_ICHANNELFILTER

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/IComponent.h"

#include <map>

namespace WireCell {

    /**
       A channel filter mutates digitized waveforms from channels.
     */
    class IChannelFilter : public WireCell::IComponent<IChannelFilter> {
       public:
        virtual ~IChannelFilter();

        typedef Waveform::realseq_t signal_t;
        typedef std::map<int, signal_t> channel_signals_t;

        /** Filter in place the signal `sig` from given
         * `channel`. Return a channel mask map with any tick-level
         * masking that may be applied later. */
        virtual Waveform::ChannelMaskMap apply(int channel, signal_t& sig) const = 0;

        /** Filter in place a group of signals together. Return a
         * channel mask map with any tick-level masking that may be
         * applied later.*/
        virtual Waveform::ChannelMaskMap apply(channel_signals_t& chansig) const = 0;
    };

}  // namespace WireCell

#endif
