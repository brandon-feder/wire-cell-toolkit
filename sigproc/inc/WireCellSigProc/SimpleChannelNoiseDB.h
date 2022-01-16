#ifndef WIRECELLSIGPROC_SIMPLECHANNELNOISEDB
#define WIRECELLSIGPROC_SIMPLECHANNELNOISEDB

#include "WireCellIface/IChannelNoiseDatabase.h"

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Units.h"

#include <vector>
#include <tuple>
#include <unordered_map>
#include <memory>

namespace WireCell {
    namespace SigProc {

        class SimpleChannelNoiseDB : public WireCell::IChannelNoiseDatabase {
           public:
            /// Create a simple channel noise DB for digitized waveforms
            /// with the given size and number of samples.  Default is for
            /// microboone.
            SimpleChannelNoiseDB(double tick = 0.5 * units::us, int nsamples = 9600);
            virtual ~SimpleChannelNoiseDB();

            // IChannelNoiseDatabase
            virtual double sample_time() const { return m_tick; }

            virtual double nominal_baseline(int channel) const;
            virtual double gain_correction(int channel) const;
            virtual double response_offset(int channel) const;
            virtual int pad_window_front(int channel) const;
            virtual int pad_window_back(int channel) const;

            virtual float coherent_nf_decon_limit(int channel) const;
            virtual float coherent_nf_decon_lf_cutoff(int channel) const;
            virtual float coherent_nf_decon_limit1(int channel) const;
            virtual float coherent_nf_adc_limit(int channel) const;
            virtual float coherent_nf_protection_factor(int channel) const;
            virtual float coherent_nf_min_adc_limit(int channel) const;
            virtual float coherent_nf_roi_min_max_ratio(int channel) const;

            virtual double min_rms_cut(int channel) const;
            virtual double max_rms_cut(int channel) const;

            virtual const filter_t& rcrc(int channel) const;
            virtual const filter_t& config(int channel) const;
            virtual const filter_t& noise(int channel) const;
            virtual const filter_t& response(int channel) const;

            virtual std::vector<channel_group_t> coherent_channels() const { return m_channel_groups; }
            virtual channel_group_t bad_channels() const { return m_bad_channels; }

            // concrete helper methods

            /// Set the size and number of samples of a channel's
            /// waveform, default is for microboone.
            ///
            /// Warning: calling this will reset any settings for
            /// gains+shaping and rcrc as they depend on knowing the
            /// sampling.
            void set_sampling(double tick = 0.5 * units::us, int nsamples = 9600);

            /// Set nominal baseline in units of ADC (eg uB is -2048 for U/V, -400 for W)
            void set_nominal_baseline(const std::vector<int>& channels, double baseline);

            /// Set gain/shaping corrections for cnofig_correction.  Gains
            /// are assumed to be in mV/fC.  Shaping times should be in
            /// the system of units.  Defaults are microboone (but you
            /// need to give channels).
            void set_gains_shapings(const std::vector<int>& channels, double from_gain_mVfC = 7.8,
                                    double to_gain_mVfC = 14.0, double from_shaping = 1.0 * units::us,
                                    double to_shaping = 2.0 * units::us);

            /// Set a response offset for the given set of channels.
            void set_response_offset(const std::vector<int>& channels, double offset);
            void set_pad_window_front(const std::vector<int>& channels, int pad_f);
            void set_pad_window_back(const std::vector<int>& channels, int pad_b);

            void set_coherent_nf_decon_limit(const std::vector<int>& channels, float decon_limit);
            void set_coherent_nf_decon_lf_cutoff(const std::vector<int>& channels, float decon_lf_cutoff);
            void set_coherent_nf_decon_limit1(const std::vector<int>& channels, float decon_limit);
            void set_coherent_nf_adc_limit(const std::vector<int>& channels, float adc_limit);
            void set_coherent_nf_protection_factor(const std::vector<int>& channels, float protection_factor);
            void set_coherent_nf_min_adc_limit(const std::vector<int>& channels, float min_adc_limit);
            void set_coherent_nf_roi_min_max_ratio(const std::vector<int>& channels, float roi_min_max_ratio);

            void set_min_rms_cut(const std::vector<int>& channels, double min_rms);
            void set_min_rms_cut_one(int channel, double min_rms);
            void set_max_rms_cut(const std::vector<int>& channels, double max_rms);
            void set_max_rms_cut_one(int channel, double max_rms);

            /// Set the RC+RC time constant in the system of units for the
            /// digitization sample time ("tick").  Default is for microboone.
            void set_rcrc_constant(const std::vector<int>& channels, double rcrc = 2000.0);

            /// Set a detector response spectrum for the set of channels
            void set_response(const std::vector<int>& channels, const filter_t& spectrum);

            /// Set a constant scaling to a band covering the given
            /// frequency bins (inclusively) for the given channels.
            /// Frequency bin "i" is from i*f to (i+1)*f where f is
            /// 1.0/(nsamples*tick).  The largest meaningful frequency bin
            /// is nsamples/2.  The frequency band is *inclusive* of both
            /// min and max frequency bins.  Note, it's up to caller to
            /// appropriately segment multiple masks across multiple
            /// channels.  For any given channel, last call to this method
            /// wins.
            typedef std::tuple<double, int, int> mask_t;
            typedef std::vector<mask_t> multimask_t;
            void set_filter(const std::vector<int>& channels, const multimask_t& mask);

            /// Set the channel groups
            void set_channel_groups(const std::vector<channel_group_t>& channel_groups);

            /// Set "bad" channels.
            void set_bad_channels(const channel_group_t& bc);

           private:
            double m_tick;
            int m_nsamples;

            double m_default_baseline, m_default_gain, m_default_offset;
            double m_default_min_rms, m_default_max_rms;
            int m_default_pad_f, m_default_pad_b;
            float m_default_decon_limit, m_default_decon_lf_cutoff, m_default_adc_limit, m_default_decon_limit1,
                m_default_protection_factor, m_default_min_adc_limit, m_default_roi_min_max_ratio;

            std::vector<double> m_baseline, m_gain, m_offset, m_min_rms, m_max_rms;
            std::vector<int> m_pad_f, m_pad_b;
            std::vector<float> m_decon_limit, m_decon_lf_cutoff, m_adc_limit, m_decon_limit1, m_protection_factor,
                m_min_adc_limit, m_roi_min_max_ratio;

            typedef std::shared_ptr<filter_t> shared_filter_t;
            typedef std::vector<shared_filter_t> filter_vector_t;
            filter_vector_t m_rcrc, m_config, m_masks, m_response;
            shared_filter_t m_default_filter;
            shared_filter_t m_default_response;

            mutable std::unordered_map<int, int> m_ch2ind;
            int chind(int ch) const;

            const IChannelNoiseDatabase::filter_t& get_filter(int channel, const filter_vector_t& fv) const;

            std::vector<channel_group_t> m_channel_groups;
            channel_group_t m_bad_channels;
        };
    }  // namespace SigProc

}  // namespace WireCell

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
