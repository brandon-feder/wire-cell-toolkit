#ifndef WIRECELLSIGPROC_OMNIBUSSIGPROC
#define WIRECELLSIGPROC_OMNIBUSSIGPROC

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IWaveform.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Array.h"
#include "WireCellAux/Logger.h"

#include <list>

namespace WireCell {
    namespace SigProc {

        class SignalROI;  // forward declaration
        class OmnibusSigProc : public Aux::Logger,
                               public WireCell::IFrameFilter, public WireCell::IConfigurable {
           public:
            OmnibusSigProc(
                const std::string& anode_tn = "AnodePlane", const std::string& per_chan_resp_tn = "PerChannelResponse",
                const std::string& field_response = "FieldResponse", double fine_time_offset = 0.0 * units::microsecond,
                double coarse_time_offset = -8.0 * units::microsecond, const std::string& elecresponse_tn = "ColdElec",
                double gain = 14.0 * units::mV / units::fC, double shaping_time = 2.2 * units::microsecond,
                double inter_gain = 1.2, double ADC_mV = 4096 / (2000. * units::mV), float th_factor_ind = 3,
                float th_factor_col = 5, int pad = 5, float asy = 0.1, int rebin = 6, double l_factor = 3.5,
                double l_max_th = 10000, double l_factor1 = 0.7, int l_short_length = 3, int l_jump_one_bin = 0,
                double r_th_factor = 3.0, double r_fake_signal_low_th = 500, double r_fake_signal_high_th = 1000,
                double r_fake_signal_low_th_ind_factor = 1.0, double r_fake_signal_high_th_ind_factor = 1.0,
                int r_pad = 5, int r_break_roi_loop = 2, double r_th_peak = 3.0, double r_sep_peak = 6.0,
                double r_low_peak_sep_threshold_pre = 1200, int r_max_npeaks = 200, double r_sigma = 2.0,
                double r_th_percent = 0.1, std::vector<int> process_planes = {0, 1, 2}, int charge_ch_offset = 10000,
                const std::string& wiener_tag = "wiener", const std::string& wiener_threshold_tag = "threshold",
                const std::string& decon_charge_tag = "decon_charge", const std::string& gauss_tag = "gauss",
                bool use_roi_debug_mode = false, bool use_roi_refinement = true, const std::string& tight_lf_tag = "tight_lf",
                const std::string& loose_lf_tag = "loose_lf", const std::string& cleanup_roi_tag = "cleanup_roi",
                const std::string& break_roi_loop1_tag = "break_roi_1st",
                const std::string& break_roi_loop2_tag = "break_roi_2nd",
                const std::string& shrink_roi_tag = "shrink_roi", const std::string& extend_roi_tag = "extend_roi",
                const std::string& mp3_roi_tag = "mp3_roi", const std::string& mp2_roi_tag = "mp2_roi");
            virtual ~OmnibusSigProc();

            virtual bool operator()(const input_pointer& in, output_pointer& out);

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            // convert data into Eigen Matrix
            void load_data(const input_pointer& in, int plane);

            // deconvolution
            void decon_2D_init(int plane);  // main decon code
            void decon_2D_ROI_refine(int plane);
            void decon_2D_tightROI(int plane);
            void decon_2D_tighterROI(int plane);
            void decon_2D_looseROI(int plane);
            void decon_2D_hits(int plane);
            void decon_2D_charge(int plane);

            void decon_2D_looseROI_debug_mode(int plane);

            // save data into the out frame and collect the indices
            void save_data(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                           const std::vector<float>& perwire_rmses, IFrame::trace_summary_t& threshold);

            // save ROI into the out frame (set use_roi_debug_mode=true)
            void save_roi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                          std::vector<std::list<SignalROI*> >& roi_channel_list);

            // save Multi-Plane ROI into the out frame (set use_roi_debug_mode=true)
            // mp_rois: osp-chid, start -> start, end
            void save_mproi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                            std::multimap<std::pair<int, int>, std::pair<int, int> > mp_rois);

            void save_ext_roi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                              std::vector<std::list<SignalROI*> >& roi_channel_list);

            // initialize the overall response function ...
            void init_overall_response(IFrame::pointer frame);

            void restore_baseline(WireCell::Array::array_xxf& arr);

            // This little struct is used to map between WCT channel idents
            // and internal OmnibusSigProc wire/channel numbers.  See
            // m_channel_map and m_channel_range below.
            struct OspChan {
                int channel;  // between 0 and nwire_u+nwire_v+nwire_w-1
                int wire;     // between 0 and nwire_{u,v,w,}-1 depending on plane
                int plane;    // 0,1,2
                int ident;    // wct ident, opaque non-negative number.  set in wires geom file
                OspChan(int c = -1, int w = -1, int p = -1, int id = -1)
                  : channel(c)
                  , wire(w)
                  , plane(p)
                  , ident(id)
                {
                }
                std::string str() const;
            };

            // find if neighbor channels hare masked.
            bool masked_neighbors(const std::string& cmname, OspChan& ochan, int nnn);

            // Anode plane for geometry
            std::string m_anode_tn;
            IAnodePlane::pointer m_anode;
            std::string m_per_chan_resp;
            std::string m_field_response;

            // Overall time offset
            double m_fine_time_offset;    // must be positive, between 0-0.5 us, shift the response function to earlier
                                          // time --> shift the deconvoluted signal to a later time
            double m_coarse_time_offset;  // additional coarse time shift ...
            double m_intrinsic_time_offset;
            int m_wire_shift[3];

            // bins
            double m_period;
            int m_nticks;
            int m_fft_flag;
            int m_fft_nwires[3], m_pad_nwires[3];
            int m_fft_nticks, m_pad_nticks;

            // gain, shaping time, other applification factors
            std::string m_elecresponse_tn;
            std::shared_ptr<IWaveform> m_elecresponse;
            double m_gain, m_shaping_time;
            double m_inter_gain, m_ADC_mV;

            // some parameters for ROI creating
            float m_th_factor_ind;
            float m_th_factor_col;
            int m_pad;
            float m_asy;
            int m_rebin;
            double m_l_factor;
            double m_l_max_th;
            double m_l_factor1;
            int m_l_short_length;
            int m_l_jump_one_bin;

            // ROI_refinement
            double m_r_th_factor;
            double m_r_fake_signal_low_th;
            double m_r_fake_signal_high_th;
            double m_r_fake_signal_low_th_ind_factor;
            double m_r_fake_signal_high_th_ind_factor;
            int m_r_pad;
            int m_r_break_roi_loop;
            double m_r_th_peak;
            double m_r_sep_peak;
            double m_r_low_peak_sep_threshold_pre;
            int m_r_max_npeaks;
            double m_r_sigma;
            double m_r_th_percent;

            // specify the planes to process
            std::vector<int> m_process_planes;

            // fixme: this is apparently not used:
            // channel offset
            int m_charge_ch_offset;

            // CAUTION: this class was originally written for microboone
            // which is degenerate in how wires and channels may be
            // numbered.  DUNE APAs do not have a one-to-one nor simple
            // mapping between a sequenctial "channel number", "wire number"
            // and "channel ident".  In OSP and the related ROI code,
            // wherever you see a "wire" number, it counts the segment-0
            // wire segment in order of increasing pitch and assuming one
            // logical plane so it will wrap around for two-faced APAs like
            // in DUNE.  An OSP "channel" number goes from 0 to
            // nwire_u+nwire_v+nwire_w-1 over the entire APA.  A WCT "ident"
            // number is totally opaque, you can't assume anything about it
            // except that it is nonnegative.
            int m_nwires[3];

            // ROI->chid() -> wct ch ident
            std::map<int, int> m_roi_ch_ch_ident;

            // Need to go from WCT channel ident to {OSP channel, wire and plane}
            std::map<int, OspChan> m_channel_map;

            // Need to go from OSP plane to iterable {OSP channel an wire and WCT ident}
            std::vector<OspChan> m_channel_range[3];

            // This is the input channel mask map but converted to OSP
            // channel number indices.  See above.  This is NOT a direct
            // copy from the IFrame.  It's reindexed by osp channel, not WCT
            // channel ident!
            Waveform::ChannelMaskMap m_cmm;

            // Per-plane temporary working arrays.  Each column is one tick,
            // each row is indexec by an "OSP wire" number
            Array::array_xxf m_r_data[3];
            Array::array_xxc m_c_data[3];

            // average overall responses
            std::vector<Waveform::realseq_t> overall_resp[3];

            // tag name for traces
            std::string m_wiener_tag;
            std::string m_wiener_threshold_tag;
            std::string m_decon_charge_tag;
            std::string m_gauss_tag;
            std::string m_frame_tag;

            bool m_use_roi_debug_mode;
            bool m_use_roi_refinement;
            std::string m_tight_lf_tag;
            std::string m_loose_lf_tag;
            std::string m_cleanup_roi_tag;
            std::string m_break_roi_loop1_tag;
            std::string m_break_roi_loop2_tag;
            std::string m_shrink_roi_tag;
            std::string m_extend_roi_tag;

            bool m_use_multi_plane_protection;
            std::string m_mp3_roi_tag;
            std::string m_mp2_roi_tag;

            bool m_isWrapped;

            // If true, safe output as a sparse frame.  Traces will only
            // cover segments of waveforms which have non-zero signal
            // samples.
            bool m_sparse;

            size_t m_count{0};

        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
