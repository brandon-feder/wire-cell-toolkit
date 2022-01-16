/**


 */

#ifndef WIRECELL_IDECONVOLUTION
#define WIRECELL_IDECONVOLUTION

namespace WireCell {

    /**
       A deconvolution applies the inverse of a convolution
     */
    class IDeconvolution : public WireCell::IComponent<IDeconvolution> {
       public:
        virtual ~IDeconvolution();

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

#endif  //  WIRECELL_IDECONVOLUTION
