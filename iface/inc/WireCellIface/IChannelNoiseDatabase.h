#ifndef WIRECELL_ICHANNELNOISEDATABASE
#define WIRECELL_ICHANNELNOISEDATABASE

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/IComponent.h"

#include <vector>

namespace WireCell {

    // FIXME: this needs to be simplified into a Noise Subtraction
    // Service interface.  All these details need to be subsumed into
    // an implementation!
    class IChannelNoiseDatabase : public WireCell::IComponent<IChannelNoiseDatabase> {
       public:
        /// The data type for all frequency-space, multiplicative filters.
        typedef WireCell::Waveform::compseq_t filter_t;
        typedef std::vector<int> channel_group_t;

        virtual ~IChannelNoiseDatabase();

        /// FIXME: how to handle state changes?

        /// Return the time-domain sample period (time in system of
        /// units) which is was used in producing the filter
        /// response spectral functions (filter_t).
        ///
        /// Warning: take care that the number of frequency samples
        /// (filter_t::size()) is fixed and may not match the number
        /// of ticks in your waveform.
        virtual double sample_time() const = 0;

        /// Return a nominal baseline correction (additive offset)
        virtual double nominal_baseline(int channel) const = 0;

        /// Return simple gain correction (a multiplicative, unitless
        /// scaling) to apply to a given channel.
        virtual double gain_correction(int channel) const = 0;

        /// Return a time offset associated with the response().
        virtual double response_offset(int channel) const = 0;

        // return the protected window padding in the front
        virtual int pad_window_front(int channel) const = 0;
        // return the protected window padding in the back
        virtual int pad_window_back(int channel) const = 0;

        virtual float coherent_nf_decon_limit(int channel) const = 0;
        virtual float coherent_nf_decon_lf_cutoff(int channel) const = 0;
        virtual float coherent_nf_adc_limit(int channel) const = 0;
        virtual float coherent_nf_decon_limit1(int channel) const = 0;
        virtual float coherent_nf_protection_factor(int channel) const = 0;
        virtual float coherent_nf_min_adc_limit(int channel) const = 0;
        virtual float coherent_nf_roi_min_max_ratio(int channel) const = 0;

        // return the min rms cut for a channel
        virtual double min_rms_cut(int channel) const = 0;
        // return the max rms cut for a channel
        virtual double max_rms_cut(int channel) const = 0;

        /// Return the filter for the RC+RC coupling response function.
        virtual const filter_t& rcrc(int channel) const = 0;

        /// Return the filter to correct any wrongly configured channels.
        virtual const filter_t& config(int channel) const = 0;

        /// Return the filter to attenuate noise.
        virtual const filter_t& noise(int channel) const = 0;

        /// A nominal detector response spectrum for a given channel.
        virtual const filter_t& response(int channel) const = 0;

        /// Return channel grouping for coherent noise subtraction
        virtual std::vector<channel_group_t> coherent_channels() const = 0;

        /// Return channels which are considered a'priori "bad".
        virtual channel_group_t bad_channels() const { return channel_group_t(); }

        /// Return channels which are considered a'priori "misconfigured".
        virtual channel_group_t miscfg_channels() const { return channel_group_t(); }
    };

}  // namespace WireCell

#endif
