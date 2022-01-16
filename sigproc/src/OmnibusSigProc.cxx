#include "WireCellSigProc/OmnibusSigProc.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/FFTBestLength.h"
#include "WireCellUtil/Waveform.h"

#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include "WireCellIface/IFieldResponse.h"
#include "WireCellIface/IFilterWaveform.h"
#include "WireCellIface/IChannelResponse.h"

#include "ROI_formation.h"
#include "ROI_refinement.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(OmnibusSigProc, WireCell::SigProc::OmnibusSigProc,
                 WireCell::INamed,
                 WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace WireCell;

using namespace WireCell::SigProc;

OmnibusSigProc::OmnibusSigProc(
    const std::string& anode_tn, const std::string& per_chan_resp_tn, const std::string& field_response,
    double fine_time_offset, double coarse_time_offset, const std::string& elecresponse_tn, double gain,
    double shaping_time, double inter_gain, double ADC_mV, float th_factor_ind, float th_factor_col, int pad, float asy,
    int rebin, double l_factor, double l_max_th, double l_factor1, int l_short_length, int l_jump_one_bin,
    double r_th_factor, double r_fake_signal_low_th, double r_fake_signal_high_th,
    double r_fake_signal_low_th_ind_factor, double r_fake_signal_high_th_ind_factor, int r_pad, int r_break_roi_loop,
    double r_th_peak, double r_sep_peak, double r_low_peak_sep_threshold_pre, int r_max_npeaks, double r_sigma,
    double r_th_percent, std::vector<int> process_planes, int charge_ch_offset, const std::string& wiener_tag,
    const std::string& wiener_threshold_tag, const std::string& decon_charge_tag, const std::string& gauss_tag,
    bool use_roi_debug_mode, bool use_roi_refinement, const std::string& tight_lf_tag, const std::string& loose_lf_tag,
    const std::string& cleanup_roi_tag, const std::string& break_roi_loop1_tag, const std::string& break_roi_loop2_tag,
    const std::string& shrink_roi_tag, const std::string& extend_roi_tag, const std::string& mp3_roi_tag,
    const std::string& mp2_roi_tag)
  : Aux::Logger("OmnibusSigProc", "sigproc")
  , m_anode_tn(anode_tn)
  , m_per_chan_resp(per_chan_resp_tn)
  , m_field_response(field_response)
  , m_fine_time_offset(fine_time_offset)
  , m_coarse_time_offset(coarse_time_offset)
  , m_period(0)
  , m_nticks(0)
  , m_fft_flag(0)
  , m_elecresponse_tn(elecresponse_tn)
  , m_gain(gain)
  , m_shaping_time(shaping_time)
  , m_inter_gain(inter_gain)
  , m_ADC_mV(ADC_mV)
  , m_th_factor_ind(th_factor_ind)
  , m_th_factor_col(th_factor_col)
  , m_pad(pad)
  , m_asy(asy)
  , m_rebin(rebin)
  , m_l_factor(l_factor)
  , m_l_max_th(l_max_th)
  , m_l_factor1(l_factor1)
  , m_l_short_length(l_short_length)
  , m_l_jump_one_bin(l_jump_one_bin)
  , m_r_th_factor(r_th_factor)
  , m_r_fake_signal_low_th(r_fake_signal_low_th)
  , m_r_fake_signal_high_th(r_fake_signal_high_th)
  , m_r_fake_signal_low_th_ind_factor(r_fake_signal_low_th_ind_factor)
  , m_r_fake_signal_high_th_ind_factor(r_fake_signal_high_th_ind_factor)
  , m_r_pad(r_pad)
  , m_r_break_roi_loop(r_break_roi_loop)
  , m_r_th_peak(r_th_peak)
  , m_r_sep_peak(r_sep_peak)
  , m_r_low_peak_sep_threshold_pre(r_low_peak_sep_threshold_pre)
  , m_r_max_npeaks(r_max_npeaks)
  , m_r_sigma(r_sigma)
  , m_r_th_percent(r_th_percent)
  , m_process_planes(process_planes)
  , m_charge_ch_offset(charge_ch_offset)
  , m_wiener_tag(wiener_tag)
  , m_wiener_threshold_tag(wiener_threshold_tag)
  , m_decon_charge_tag(decon_charge_tag)
  , m_gauss_tag(gauss_tag)
  , m_frame_tag("sigproc")
  , m_use_roi_debug_mode(use_roi_debug_mode)
  , m_use_roi_refinement(use_roi_refinement)
  , m_tight_lf_tag(tight_lf_tag)
  , m_loose_lf_tag(loose_lf_tag)
  , m_cleanup_roi_tag(cleanup_roi_tag)
  , m_break_roi_loop1_tag(break_roi_loop1_tag)
  , m_break_roi_loop2_tag(break_roi_loop2_tag)
  , m_shrink_roi_tag(shrink_roi_tag)
  , m_extend_roi_tag(extend_roi_tag)
  , m_use_multi_plane_protection(false)
  , m_mp3_roi_tag(mp3_roi_tag)
  , m_mp2_roi_tag(mp2_roi_tag)
  , m_isWrapped(false)
  , m_sparse(false)
{
    // get wires for each plane

    // std::cout << m_anode->channels().size() << " " << nwire_u << " " << nwire_v << " " << nwire_w << std::endl;
}

OmnibusSigProc::~OmnibusSigProc() {}

std::string WireCell::SigProc::OmnibusSigProc::OspChan::str() const
{
    std::stringstream ss;
    ss << "OspChan<c:" << channel << ",w:" << wire << ",p:" << plane << ",i:" << ident << ">";
    return ss.str();
}

void OmnibusSigProc::configure(const WireCell::Configuration& config)
{
    m_sparse = get(config, "sparse", false);

    m_fine_time_offset = get(config, "ftoffset", m_fine_time_offset);
    m_coarse_time_offset = get(config, "ctoffset", m_coarse_time_offset);
    m_anode_tn = get(config, "anode", m_anode_tn);

    // m_nticks = get(config,"nticks",m_nticks);
    if (!config["nticks"].isNull()) {
        log->warn("no setting \"nticks\", ignoring value {}", config["nticks"].asInt());
    }
    // m_period = get(config,"period",m_period);
    if (!config["period"].isNull()) {
        log->warn("no setting \"period\", ignoring value {}", config["period"].asDouble());
    }

    m_fft_flag = get(config, "fft_flag", m_fft_flag);
    if (m_fft_flag) {
      m_fft_flag = 0;
      log->warn("fft_flag option is broken, will use native array sizes");
    }
    m_elecresponse_tn = get(config, "elecresponse", m_elecresponse_tn);
    m_gain = get(config, "gain", m_gain);
    m_shaping_time = get(config, "shaping", m_shaping_time);
    m_inter_gain = get(config, "postgain", m_inter_gain);
    m_ADC_mV = get(config, "ADC_mV", m_ADC_mV);

    m_per_chan_resp = get(config, "per_chan_resp", m_per_chan_resp);
    m_field_response = get(config, "field_response", m_field_response);

    m_th_factor_ind = get(config, "troi_ind_th_factor", m_th_factor_ind);
    m_th_factor_col = get(config, "troi_col_th_factor", m_th_factor_col);
    m_pad = get(config, "troi_pad", m_pad);
    m_asy = get(config, "troi_asy", m_asy);
    m_rebin = get(config, "lroi_rebin", m_rebin);
    m_l_factor = get(config, "lroi_th_factor", m_l_factor);
    m_l_max_th = get(config, "lroi_max_th", m_l_max_th);
    m_l_factor1 = get(config, "lroi_th_factor1", m_l_factor1);
    m_l_short_length = get(config, "lroi_short_length", m_l_short_length);
    m_l_jump_one_bin = get(config, "lroi_jump_one_bin", m_l_jump_one_bin);

    m_r_th_factor = get(config, "r_th_factor", m_r_th_factor);
    m_r_fake_signal_low_th = get(config, "r_fake_signal_low_th", m_r_fake_signal_low_th);
    m_r_fake_signal_high_th = get(config, "r_fake_signal_high_th", m_r_fake_signal_high_th);
    m_r_fake_signal_low_th_ind_factor =
        get(config, "r_fake_signal_low_th_ind_factor", m_r_fake_signal_low_th_ind_factor);
    m_r_fake_signal_high_th_ind_factor =
        get(config, "r_fake_signal_high_th_ind_factor", m_r_fake_signal_high_th_ind_factor);
    m_r_pad = get(config, "r_pad", m_r_pad);
    m_r_break_roi_loop = get(config, "r_break_roi_loop", m_r_break_roi_loop);
    m_r_th_peak = get(config, "r_th_peak", m_r_th_peak);
    m_r_sep_peak = get(config, "r_sep_peak", m_r_sep_peak);
    m_r_low_peak_sep_threshold_pre = get(config, "r_low_peak_sep_threshold_pre", m_r_low_peak_sep_threshold_pre);
    m_r_max_npeaks = get(config, "r_max_npeaks", m_r_max_npeaks);
    m_r_sigma = get(config, "r_sigma", m_r_sigma);
    m_r_th_percent = get(config, "r_th_percent", m_r_th_percent);

    if (config.isMember("process_planes")) {
        m_process_planes.clear();
        for (auto jplane : config["process_planes"]) {
            m_process_planes.push_back(jplane.asInt());
        }
    }

    m_charge_ch_offset = get(config, "charge_ch_offset", m_charge_ch_offset);

    m_wiener_tag = get(config, "wiener_tag", m_wiener_tag);
    m_wiener_threshold_tag = get(config, "wiener_threshold_tag", m_wiener_threshold_tag);
    m_decon_charge_tag = get(config, "decon_charge_tag", m_decon_charge_tag);
    m_gauss_tag = get(config, "gauss_tag", m_gauss_tag);
    m_frame_tag = get(config, "frame_tag", m_frame_tag);

    m_use_roi_debug_mode = get(config, "use_roi_debug_mode", m_use_roi_debug_mode);
    m_use_roi_refinement = get(config, "use_roi_refinement", m_use_roi_refinement);
    m_tight_lf_tag = get(config, "tight_lf_tag", m_tight_lf_tag);
    m_loose_lf_tag = get(config, "loose_lf_tag", m_loose_lf_tag);
    m_cleanup_roi_tag = get(config, "cleanup_roi_tag", m_cleanup_roi_tag);
    m_break_roi_loop1_tag = get(config, "break_roi_loop1_tag", m_break_roi_loop1_tag);
    m_break_roi_loop2_tag = get(config, "break_roi_loop2_tag", m_break_roi_loop2_tag);
    m_shrink_roi_tag = get(config, "shrink_roi_tag", m_shrink_roi_tag);
    m_extend_roi_tag = get(config, "extend_roi_tag", m_extend_roi_tag);

    m_use_multi_plane_protection = get<bool>(config, "use_multi_plane_protection", m_use_multi_plane_protection);
    m_mp3_roi_tag = get(config, "mp3_roi_tag", m_mp3_roi_tag);
    m_mp2_roi_tag = get(config, "mp2_roi_tag", m_mp2_roi_tag);

    m_isWrapped = get<bool>(config, "isWrapped", m_isWrapped);

    // this throws if not found
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);

    //
    m_elecresponse = Factory::find_tn<IWaveform>(m_elecresponse_tn);

    // Build up the channel map.  The OSP channel must run contiguously
    // first up the U, then V, then W "wires".  Ie, face-major order,
    // but we have plane-major order so make a temporary collection.
    IChannel::vector plane_channels[3];
    std::stringstream ss;
    ss << "internal channel map for tags: gauss:\"" << m_gauss_tag << "\", wiener:\"" << m_wiener_tag
       << "\", frame:\"" << m_frame_tag << "\"\n";

    // fixme: this loop is now available as Aux::plane_channels()
    for (auto face : m_anode->faces()) {
        if (!face) {   // A null face means one sided AnodePlane.
            continue;  // Can be "back" or "front" face.
        }
        for (auto plane : face->planes()) {
            int plane_index = plane->planeid().index();
            auto& pchans = plane_channels[plane_index];
            // These IChannel vectors are ordered in same order as wire-in-plane.
            const auto& ichans = plane->channels();
            // Append
            pchans.reserve(pchans.size() + ichans.size());
            pchans.insert(pchans.end(), ichans.begin(), ichans.end());
            ss << "\tpind" << plane_index << " "
               << "aid" << m_anode->ident() << " "
               << "fid" << face->ident() << " "
               << "pid" << plane->ident() << " "
               << "cid" << ichans.front()->ident() << " -> cid" << ichans.back()->ident() << ", "
               << "cind" << ichans.front()->index() << " -> cind" << ichans.back()->index() << ", "
               << "(n=" << pchans.size() << ")\n";
        }
    }
    log->debug(ss.str());

    int osp_channel_number = 0;
    for (int iplane = 0; iplane < 3; ++iplane) {
        m_nwires[iplane] = plane_channels[iplane].size();
        int osp_wire_number = 0;
        for (auto ichan : plane_channels[iplane]) {
            const int wct_chan_ident = ichan->ident();
            OspChan och(osp_channel_number, osp_wire_number, iplane, wct_chan_ident);
            // std::cout << "[hyu1]chmap: " << wct_chan_ident << " " << iplane << " " << osp_channel_number << " " <<
            // osp_wire_number << std::endl;
            m_roi_ch_ch_ident[osp_channel_number] = wct_chan_ident;
            m_channel_map[wct_chan_ident] = och;     // we could save some space by storing
            m_channel_range[iplane].push_back(och);  // wct ident here instead of a whole och.
            ++osp_wire_number;
            ++osp_channel_number;
        }
    }
}

WireCell::Configuration OmnibusSigProc::default_configuration() const
{
    Configuration cfg;
    cfg["anode"] = m_anode_tn;
    cfg["ftoffset"] = m_fine_time_offset;
    cfg["ctoffset"] = m_coarse_time_offset;
    // cfg["nticks"] = m_nticks;
    // cfg["period"] = m_period;

    //cfg["fft_flag"] = m_fft_flag;
    cfg["fft_flag"] = 0;

    cfg["elecresponse"] = m_elecresponse_tn;
    cfg["gain"] = m_gain;
    cfg["shaping"] = m_shaping_time;
    cfg["inter_gain"] = m_inter_gain;
    cfg["ADC_mV"] = m_ADC_mV;

    cfg["per_chan_resp"] = m_per_chan_resp;
    cfg["field_response"] = m_field_response;

    cfg["troi_ind_th_factor"] = m_th_factor_ind;
    cfg["troi_col_th_factor"] = m_th_factor_col;
    cfg["troi_pad"] = m_pad;
    cfg["troi_asy"] = m_asy;
    cfg["lroi_rebin"] = m_rebin;
    cfg["lroi_th_factor"] = m_l_factor;
    cfg["lroi_max_th"] = m_l_max_th;
    cfg["lroi_th_factor1"] = m_l_factor1;
    cfg["lroi_short_length"] = m_l_short_length;
    cfg["lroi_jump_one_bin"] = m_l_jump_one_bin;

    cfg["r_th_factor"] = m_r_th_factor;
    cfg["r_fake_signal_low_th"] = m_r_fake_signal_low_th;
    cfg["r_fake_signal_high_th"] = m_r_fake_signal_high_th;
    cfg["r_fake_signal_low_th_ind_factor"] = m_r_fake_signal_low_th_ind_factor;
    cfg["r_fake_signal_high_th_ind_factor"] = m_r_fake_signal_high_th_ind_factor;
    cfg["r_pad"] = m_r_pad;
    cfg["r_break_roi_loop"] = m_r_break_roi_loop;
    cfg["r_th_peak"] = m_r_th_peak;
    cfg["r_sep_peak"] = m_r_sep_peak;
    cfg["r_low_peak_sep_threshold_pre"] = m_r_low_peak_sep_threshold_pre;
    cfg["r_max_npeaks"] = m_r_max_npeaks;
    cfg["r_sigma"] = m_r_sigma;
    cfg["r_th_precent"] = m_r_th_percent;

    // cfg["process_planes"] = Json::arrayValue;

    // fixme: unused?
    cfg["charge_ch_offset"] = m_charge_ch_offset;

    cfg["wiener_tag"] = m_wiener_tag;
    cfg["wiener_threshold_tag"] = m_wiener_threshold_tag;
    cfg["decon_charge_tag"] = m_decon_charge_tag;
    cfg["gauss_tag"] = m_gauss_tag;
    cfg["frame_tag"] = m_frame_tag;

    cfg["use_roi_debug_mode"] = m_use_roi_debug_mode;  // default false
    cfg["use_roi_refinement"] = m_use_roi_refinement;  // default true
    cfg["tight_lf_tag"] = m_tight_lf_tag;
    cfg["loose_lf_tag"] = m_loose_lf_tag;
    cfg["cleanup_roi_tag"] = m_cleanup_roi_tag;
    cfg["break_roi_loop1_tag"] = m_break_roi_loop1_tag;
    cfg["break_roi_loop2_tag"] = m_break_roi_loop2_tag;
    cfg["shrink_roi_tag"] = m_shrink_roi_tag;
    cfg["extend_roi_tag"] = m_extend_roi_tag;

    cfg["use_multi_plane_protection"] = m_use_multi_plane_protection;  // default false
    cfg["mp3_roi_tag"] = m_mp3_roi_tag;
    cfg["mp2_roi_tag"] = m_mp2_roi_tag;

    cfg["isWarped"] = m_isWrapped;  // default false

    cfg["sparse"] = false;

    return cfg;
}

void OmnibusSigProc::load_data(const input_pointer& in, int plane)
{
    m_r_data[plane] = Array::array_xxf::Zero(m_fft_nwires[plane], m_fft_nticks);

    auto traces = in->traces();

    auto& bad = m_cmm["bad"];
    int nbad = 0;

    for (auto trace : *traces.get()) {
        int wct_channel_ident = trace->channel();
        OspChan och = m_channel_map[wct_channel_ident];
        if (plane != och.plane) {
            continue;  // we'll catch it in another call to load_data
        }

        // fixme: this code uses tbin() but other places in this file will barf if tbin!=0.
        int tbin = trace->tbin();
        auto const& charges = trace->charge();
        const int ntbins = std::min((int) charges.size(), m_nticks);
        for (int qind = 0; qind < ntbins; ++qind) {
            m_r_data[plane](och.wire + m_pad_nwires[plane], tbin + qind) = charges[qind];
        }

        // ensure dead channels are indeed dead ...
        auto const& badch = bad.find(och.channel);
        if (badch == bad.end()) {
            continue;
        }

        auto const& binranges = badch->second;
        for (auto const& br : binranges) {
            ++nbad;
            for (int i = br.first; i != br.second; ++i) {
                m_r_data[plane](och.wire + m_pad_nwires[plane], i) = 0;
            }
        }
    }
    log->debug("plane index: {} input data identifies {} bad regions", plane, nbad);
}

// used in sparsifying below.  Could use C++17 lambdas....
static bool ispositive(float x) { return x > 0.0; }
static bool isZero(float x) { return x == 0.0; }

void OmnibusSigProc::save_data(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                               const std::vector<float>& perwire_rmses, IFrame::trace_summary_t& threshold)
{
    // reuse this temporary vector to hold charge for a channel.
    ITrace::ChargeSequence charge(m_nticks, 0.0);

    double qtot = 0.0;
    for (auto och : m_channel_range[plane]) {  // ordered by osp channel

        // Post process: zero out any negative signal and that from "bad" channels.
        // fixme: better if we move this outside of save_data().
        for (int itick = 0; itick != m_nticks; itick++) {
            const float q = m_r_data[plane](och.wire, itick);
            // charge.at(itick) = q > 0.0 ? q : 0.0;
            // charge.at(itick) = q ;
            if (m_use_roi_debug_mode)
                charge.at(itick) = q;  // debug mode: save all decons
            else
                charge.at(itick) = q > 0.0 ? q : 0.0;  // default mode: only save positive
        }
        {
            auto& bad = m_cmm["bad"];
            auto badit = bad.find(och.channel);
            if (badit != bad.end()) {
                for (auto bad : badit->second) {
                    for (int itick = bad.first; itick < bad.second; ++itick) {
                        charge.at(itick) = 0.0;
                    }
                }
            }
        }

        // debug
        for (int j = 0; j != m_nticks; j++) {
            qtot += charge.at(j);
        }

        const float thresh = perwire_rmses[och.wire];

        // actually save out
        if (m_sparse) {
            // Save waveform sparsely by finding contiguous, positive samples.
            std::vector<float>::const_iterator beg = charge.begin(), end = charge.end();
            auto i1 = std::find_if(beg, end, ispositive);  // first start
            while (i1 != end) {
                // stop at next zero or end and make little temp vector
                auto i2 = std::find_if(i1, end, isZero);
                const std::vector<float> q(i1, i2);

                // save out
                const int tbin = i1 - beg;
                SimpleTrace* trace = new SimpleTrace(och.ident, tbin, q);
                const size_t trace_index = itraces.size();
                indices.push_back(trace_index);
                itraces.push_back(ITrace::pointer(trace));
                threshold.push_back(thresh);

                // find start for next loop
                i1 = std::find_if(i2, end, ispositive);
            }
        }
        else {
            // Save the waveform densely, including zeros.
            SimpleTrace* trace = new SimpleTrace(och.ident, 0, charge);
            const size_t trace_index = itraces.size();
            indices.push_back(trace_index);
            itraces.push_back(ITrace::pointer(trace));
            threshold.push_back(thresh);
        }
    }

    // debug
    if (indices.empty()) {
        log->debug("save_data plane index: {} empty", plane);
    }
    else {
        const int nadded = indices.back() - indices.front() + 1;
        log->debug("save_data plane index: {}, Qtot={} "
                   "added {} traces to total {} indices:[{},{}]",
                   plane, qtot,
                   nadded, indices.size(), indices.front(), indices.back());
    }
}

// save ROI into the out frame
void OmnibusSigProc::save_roi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                              std::vector<std::list<SignalROI*>>& roi_channel_list)
{
    // reuse this temporary vector to hold charge for a channel.
    ITrace::ChargeSequence charge(m_nticks, 0.0);

    for (auto och : m_channel_range[plane]) {  // ordered by osp channel

        // std::cout << "[wgu] wire: " << och.wire << " roi_channel_list.size(): " << roi_channel_list.size() <<
        // std::endl;

        std::fill(charge.begin(), charge.end(), 0);

        // for (auto it = roi_channel_list.at(och.wire).begin(); it!= roi_channel_list.at(och.wire).end(); it++){
        //   SignalROI *roi =  *it;
        //   int start = roi->get_start_bin();
        //   int end = roi->get_end_bin();
        //   std::cout << "[wgu] wire: " << och.wire << " ROI: " << start << " " << end << " channel: " <<
        //   roi->get_chid() << " plane: " << roi->get_plane() << std::endl;
        // }

        int prev_roi_end = -1;
        for (auto signal_roi : roi_channel_list.at(och.wire)) {
            int start = signal_roi->get_start_bin();
            int end = signal_roi->get_end_bin();
            // if (och.wire==732)
            //   std::cout << "[wgu] OmnibusSigProc::save_roi() wire: " << och.wire << " channel: " << och.channel << "
            //   ROI: " << start << " " << end << " channel: " << signal_roi->get_chid() << " plane: " <<
            //   signal_roi->get_plane() << " max height: " << signal_roi->get_max_height() <<  std::endl;
            if (start < 0 or end < 0) continue;
            for (int i = start; i <= end; i++) {
                if (i - prev_roi_end < 2) continue;  // skip one bin for visibility of two adjacent ROIs
                charge.at(i) = 10.;                  // arbitary constant number for ROI display
            }
            prev_roi_end = end;
        }

        {
            auto& bad = m_cmm["bad"];
            auto badit = bad.find(och.channel);
            if (badit != bad.end()) {
                for (auto bad : badit->second) {
                    for (int itick = bad.first; itick < bad.second; ++itick) {
                        charge.at(itick) = 0.0;
                    }
                }
            }
        }

        // actually save out
        if (m_sparse) {
            // Save waveform sparsely by finding contiguous, positive samples.
            std::vector<float>::const_iterator beg = charge.begin(), end = charge.end();
            auto i1 = std::find_if(beg, end, ispositive);  // first start
            while (i1 != end) {
                // stop at next zero or end and make little temp vector
                auto i2 = std::find_if(i1, end, isZero);
                const std::vector<float> q(i1, i2);

                // save out
                const int tbin = i1 - beg;
                SimpleTrace* trace = new SimpleTrace(och.ident, tbin, q);
                const size_t trace_index = itraces.size();
                indices.push_back(trace_index);
                itraces.push_back(ITrace::pointer(trace));
                // if (och.wire==67) std::cout << "[wgu] och channel: " << och.channel << " wire: " << och.wire << "
                // plane: " << och.plane << " ident: " << och.ident << " tbin: " << tbin << " len: " << i2-i1 <<
                // std::endl;

                // find start for next loop
                i1 = std::find_if(i2, end, ispositive);
            }
        }
        else {
            // Save the waveform densely, including zeros.
            SimpleTrace* trace = new SimpleTrace(och.ident, 0, charge);
            const size_t trace_index = itraces.size();
            indices.push_back(trace_index);
            itraces.push_back(ITrace::pointer(trace));
        }
    }
}

void OmnibusSigProc::save_ext_roi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                                  std::vector<std::list<SignalROI*>>& roi_channel_list)
{
    // reuse this temporary vector to hold charge for a channel.
    ITrace::ChargeSequence charge(m_nticks, 0.0);

    for (auto och : m_channel_range[plane]) {  // ordered by osp channel

        // std::cout << "[wgu] wire: " << och.wire << " roi_channel_list.size(): " << roi_channel_list.size() <<
        // std::endl;

        std::fill(charge.begin(), charge.end(), 0);

        // for (auto it = roi_channel_list.at(och.wire).begin(); it!= roi_channel_list.at(och.wire).end(); it++){
        //   SignalROI *roi =  *it;
        //   int start = roi->get_start_bin();
        //   int end = roi->get_end_bin();
        //   std::cout << "[wgu] wire: " << och.wire << " ROI: " << start << " " << end << " channel: " <<
        //   roi->get_chid() << " plane: " << roi->get_plane() << std::endl;
        // }

        int prev_roi_end = -1;
        for (auto signal_roi : roi_channel_list.at(och.wire)) {
            int start = signal_roi->get_ext_start_bin();
            int end = signal_roi->get_ext_end_bin();
            // if (och.wire==732)
            //   std::cout << "[wgu] OmnibusSigProc::save_roi() wire: " << och.wire << " channel: " << och.channel << "
            //   ROI: " << start << " " << end << " channel: " << signal_roi->get_chid() << " plane: " <<
            //   signal_roi->get_plane() << " max height: " << signal_roi->get_max_height() <<  std::endl;
            if (start < 0 or end < 0) continue;
            for (int i = start; i <= end; i++) {
                if (i - prev_roi_end < 2) continue;  // skip one bin for visibility of two adjacent ROIs
                charge.at(i) = 10.;                  // arbitary constant number for ROI display
            }
            prev_roi_end = end;
        }

        {
            auto& bad = m_cmm["bad"];
            auto badit = bad.find(och.channel);
            if (badit != bad.end()) {
                for (auto bad : badit->second) {
                    for (int itick = bad.first; itick < bad.second; ++itick) {
                        charge.at(itick) = 0.0;
                    }
                }
            }
        }

        // actually save out
        if (m_sparse) {
            // Save waveform sparsely by finding contiguous, positive samples.
            std::vector<float>::const_iterator beg = charge.begin(), end = charge.end();
            auto i1 = std::find_if(beg, end, ispositive);  // first start
            while (i1 != end) {
                // stop at next zero or end and make little temp vector
                auto i2 = std::find_if(i1, end, isZero);
                const std::vector<float> q(i1, i2);

                // save out
                const int tbin = i1 - beg;
                SimpleTrace* trace = new SimpleTrace(och.ident, tbin, q);
                const size_t trace_index = itraces.size();
                indices.push_back(trace_index);
                itraces.push_back(ITrace::pointer(trace));
                // if (och.wire==67) std::cout << "[wgu] och channel: " << och.channel << " wire: " << och.wire << "
                // plane: " << och.plane << " ident: " << och.ident << " tbin: " << tbin << " len: " << i2-i1 <<
                // std::endl;

                // find start for next loop
                i1 = std::find_if(i2, end, ispositive);
            }
        }
        else {
            // Save the waveform densely, including zeros.
            SimpleTrace* trace = new SimpleTrace(och.ident, 0, charge);
            const size_t trace_index = itraces.size();
            indices.push_back(trace_index);
            itraces.push_back(ITrace::pointer(trace));
        }
    }
}

// save Multi-Plane ROI into the out frame (set use_roi_debug_mode=true)
// mp_rois: osp-chid, start -> start, end
void OmnibusSigProc::save_mproi(ITrace::vector& itraces, IFrame::trace_list_t& indices, int plane,
                                std::multimap<std::pair<int, int>, std::pair<int, int>> mp_rois)
{
    // reuse this temporary vector to hold charge for a channel.
    ITrace::ChargeSequence charge(m_nticks, 0.0);

    for (auto och : m_channel_range[plane]) {  // ordered by osp channel

        std::fill(charge.begin(), charge.end(), 0);

        for (auto signal_roi : mp_rois) {
            if (och.channel != signal_roi.first.first) continue;
            int start = signal_roi.second.first;
            int end = signal_roi.second.second;
            for (int i = start; i <= end; i++) {
                charge.at(i) = 4000.;  // arbitary constant number for ROI display
            }
        }

        {
            auto& bad = m_cmm["bad"];
            auto badit = bad.find(och.channel);
            if (badit != bad.end()) {
                for (auto bad : badit->second) {
                    for (int itick = bad.first; itick < bad.second; ++itick) {
                        charge.at(itick) = 0.0;
                    }
                }
            }
        }

        // actually save out
        if (m_sparse) {
            // Save waveform sparsely by finding contiguous, positive samples.
            std::vector<float>::const_iterator beg = charge.begin(), end = charge.end();
            auto i1 = std::find_if(beg, end, ispositive);  // first start
            while (i1 != end) {
                // stop at next zero or end and make little temp vector
                auto i2 = std::find_if(i1, end, isZero);
                const std::vector<float> q(i1, i2);

                // save out
                const int tbin = i1 - beg;
                SimpleTrace* trace = new SimpleTrace(och.ident, tbin, q);
                const size_t trace_index = itraces.size();
                indices.push_back(trace_index);
                itraces.push_back(ITrace::pointer(trace));

                // find start for next loop
                i1 = std::find_if(i2, end, ispositive);
            }
        }
        else {
            // Save the waveform densely, including zeros.
            SimpleTrace* trace = new SimpleTrace(och.ident, 0, charge);
            const size_t trace_index = itraces.size();
            indices.push_back(trace_index);
            itraces.push_back(ITrace::pointer(trace));
        }
    }
}

void OmnibusSigProc::init_overall_response(IFrame::pointer frame)
{
    m_period = frame->tick();
    {
        std::vector<int> tbins;
        for (auto trace : *frame->traces()) {
            const int tbin = trace->tbin();
            const int nbins = trace->charge().size();
            tbins.push_back(tbin);
            tbins.push_back(tbin + nbins);
        }
        auto mme = std::minmax_element(tbins.begin(), tbins.end());
        int tbinmin = *mme.first;
        int tbinmax = *mme.second;
        m_nticks = tbinmax - tbinmin;
        log->debug("OmnibusSigProc: nticks={} tbinmin={} tbinmax={}", m_nticks, tbinmin, tbinmax);

        if (m_fft_flag == 0) {
            m_fft_nticks = m_nticks;
        }
        else {
            m_fft_nticks = fft_best_length(m_nticks);
            log->debug("OmnibusSigProc: enlarge window from {} to {}", m_nticks, m_fft_nticks);
        }
        //

        m_pad_nticks = m_fft_nticks - m_nticks;
    }

    auto ifr = Factory::find_tn<IFieldResponse>(m_field_response);
    // Get full, "fine-grained" field responses defined at impact
    // positions.
    Response::Schema::FieldResponse fr = ifr->field_response();

    // Make a new data set which is the average FR
    Response::Schema::FieldResponse fravg = Response::wire_region_average(fr);

    for (int i = 0; i != 3; i++) {
        //
        if (m_fft_flag == 0) {
            m_fft_nwires[i] = m_nwires[i];
        }
        else {
            m_fft_nwires[i] = fft_best_length(m_nwires[i] + fravg.planes[0].paths.size() - 1, 1);
            log->debug("OmnibusSigProc: enlarge wire number in plane {} from {} to {}", i, m_nwires[i],
                       m_fft_nwires[i]);
        }
        m_pad_nwires[i] = (m_fft_nwires[i] - m_nwires[i]) / 2;
    }

    // since we only do FFT along time, no need to change dimension for wire ...
    const size_t fine_nticks = fft_best_length(fravg.planes[0].paths[0].current.size());
    int fine_nwires = fravg.planes[0].paths.size();

    WireCell::Waveform::compseq_t elec;
    WireCell::Binning tbins(fine_nticks, 0, fine_nticks * fravg.period);
    // Response::ColdElec ce(m_gain, m_shaping_time);
    // auto ewave = ce.generate(tbins);
    auto ewave = (*m_elecresponse).waveform_samples(tbins);
    Waveform::scale(ewave, m_inter_gain * m_ADC_mV * (-1));
    elec = Waveform::dft(ewave);

    std::complex<float> fine_period(fravg.period, 0);

    Waveform::realseq_t wfs(m_fft_nticks);
    Waveform::realseq_t ctbins(m_fft_nticks);
    for (int i = 0; i != m_fft_nticks; i++) {
        ctbins.at(i) = i * m_period;
    }

    Waveform::realseq_t ftbins(fine_nticks);
    for (size_t i = 0; i != fine_nticks; i++) {
        ftbins.at(i) = i * fravg.period;
    }

    // clear the overall response
    for (int i = 0; i != 3; i++) {
        overall_resp[i].clear();
    }

    m_intrinsic_time_offset = fr.origin / fr.speed;

    // Convert each average FR to a 2D array
    for (int iplane = 0; iplane < 3; ++iplane) {
        auto arr = Response::as_array(fravg.planes[iplane], fine_nwires, fine_nticks);

        // do FFT for response ...
        Array::array_xxc c_data = Array::dft_rc(arr, 0);
        int nrows = c_data.rows();
        int ncols = c_data.cols();

        for (int irow = 0; irow < nrows; ++irow) {
            for (int icol = 0; icol < ncols; ++icol) {
                c_data(irow, icol) = c_data(irow, icol) * elec.at(icol) * fine_period;
            }
        }

        arr = Array::idft_cr(c_data, 0);

        // figure out how to do fine ... shift (good ...)
        int fine_time_shift = m_fine_time_offset / fravg.period;
        if (fine_time_shift > 0) {
            Array::array_xxf arr1(nrows, ncols - fine_time_shift);
            arr1 = arr.block(0, 0, nrows, ncols - fine_time_shift);
            Array::array_xxf arr2(nrows, fine_time_shift);
            arr2 = arr.block(0, ncols - fine_time_shift, nrows, fine_time_shift);
            arr.block(0, 0, nrows, fine_time_shift) = arr2;
            arr.block(0, fine_time_shift, nrows, ncols - fine_time_shift) = arr1;

            // Array::array_xxf arr1(nrows,fine_time_shift);
            // arr1 = arr.block(0,0,nrows,fine_time_shift);
            // Array::array_xxf arr2(nrows,ncols-fine_time_shift);
            // arr2 = arr.block(0,fine_time_shift,nrows,ncols-fine_time_shift);
            // arr.block(0,0,nrows,ncols-fine_time_shift) = arr2;
            // arr.block(0,ncols-fine_time_shift,nrows,fine_time_shift) = arr1;
        }

        // redigitize ...
        for (int irow = 0; irow < fine_nwires; ++irow) {
            // gtemp = new TGraph();

            size_t fcount = 1;
            for (int i = 0; i != m_fft_nticks; i++) {
                double ctime = ctbins.at(i);

                if (fcount < fine_nticks)
                    while (ctime > ftbins.at(fcount)) {
                        fcount++;
                        if (fcount >= fine_nticks) break;
                    }

                if (fcount < fine_nticks) {
                    wfs.at(i) = ((ctime - ftbins.at(fcount - 1)) / fravg.period * arr(irow, fcount - 1) +
                                 (ftbins.at(fcount) - ctime) / fravg.period * arr(irow, fcount));  // / (-1);
                }
                else {
                    wfs.at(i) = 0;
                }
            }

            overall_resp[iplane].push_back(wfs);

            // wfs.clear();
        }  // loop inside wire ...

        // calculated the wire shift ...
        m_wire_shift[iplane] = (int(overall_resp[iplane].size()) - 1) / 2;

    }  //  loop over plane
}

void OmnibusSigProc::restore_baseline(Array::array_xxf& arr)
{
    for (int i = 0; i != arr.rows(); i++) {
        Waveform::realseq_t signal(arr.cols());
        int ncount = 0;
        for (int j = 0; j != arr.cols(); j++) {
            if (arr(i, j) != 0) {
                signal.at(ncount) = arr(i, j);
                ncount++;
            }
        }
        signal.resize(ncount);
        float baseline = WireCell::Waveform::median(signal);

        Waveform::realseq_t temp_signal(arr.cols());
        ncount = 0;
        for (size_t j = 0; j != signal.size(); j++) {
            if (fabs(signal.at(j) - baseline) < 500) {
                temp_signal.at(ncount) = signal.at(j);
                ncount++;
            }
        }
        temp_signal.resize(ncount);

        baseline = WireCell::Waveform::median(temp_signal);

        for (int j = 0; j != arr.cols(); j++) {
            if (arr(i, j) != 0) arr(i, j) -= baseline;
        }
    }
}

void OmnibusSigProc::decon_2D_init(int plane)
{
    // data part ...
    // first round of FFT on time
    m_c_data[plane] = Array::dft_rc(m_r_data[plane], 0);

    // now apply the ch-by-ch response ...
    if (!m_per_chan_resp.empty()) {
        log->debug("OmnibusSigProc: applying ch-by-ch electronics response correction");
        auto cr = Factory::find_tn<IChannelResponse>(m_per_chan_resp);
        auto cr_bins = cr->channel_response_binning();
        if (cr_bins.binsize() != m_period) {
            log->critical("OmnibusSigProc::decon_2D_init: channel response size mismatch");
            THROW(ValueError() << errmsg{"OmnibusSigProc::decon_2D_init: channel response size mismatch"});
        }
        // starndard electronics response ...
        // WireCell::Binning tbins(m_nticks, 0-m_period/2., m_nticks*m_period-m_period/2.);
        // Response::ColdElec ce(m_gain, m_shaping_time);

        // temporary hack ...
        // float scaling = 1./(1e-9*0.5/1.13312);
        // WireCell::Binning tbins(m_nticks, (-5-0.5)*m_period, (m_nticks-5-0.5)*m_period-m_period);
        // Response::ColdElec ce(m_gain*scaling, m_shaping_time);
        //// this is moved into wirecell.sigproc.main production of
        //// microboone-channel-responses-v1.json.bz2
        WireCell::Binning tbins(m_fft_nticks, cr_bins.min(), cr_bins.min() + m_fft_nticks * m_period);
        // Response::ColdElec ce(m_gain, m_shaping_time);

        // const auto ewave = ce.generate(tbins);
        auto ewave = (*m_elecresponse).waveform_samples(tbins);
        const WireCell::Waveform::compseq_t elec = Waveform::dft(ewave);

        for (auto och : m_channel_range[plane]) {
            // const auto& ch_resp = cr->channel_response(och.ident);
            Waveform::realseq_t tch_resp = cr->channel_response(och.ident);
            tch_resp.resize(m_fft_nticks, 0);
            const WireCell::Waveform::compseq_t ch_elec = Waveform::dft(tch_resp);

            const int irow = och.wire + m_pad_nwires[plane];
            for (int icol = 0; icol != m_c_data[plane].cols(); icol++) {
                const auto four = ch_elec.at(icol);
                if (std::abs(four) != 0) {
                    m_c_data[plane](irow, icol) *= elec.at(icol) / four;
                }
                else {
                    m_c_data[plane](irow, icol) = 0;
                }
            }
        }
    }

    // second round of FFT on wire
    m_c_data[plane] = Array::dft_cc(m_c_data[plane], 1);

    // response part ...
    Array::array_xxf r_resp = Array::array_xxf::Zero(m_r_data[plane].rows(), m_fft_nticks);
    for (size_t i = 0; i != overall_resp[plane].size(); i++) {
        for (int j = 0; j != m_fft_nticks; j++) {
            r_resp(i, j) = overall_resp[plane].at(i).at(j);
        }
    }

    // do first round FFT on the resposne on time
    Array::array_xxc c_resp = Array::dft_rc(r_resp, 0);
    // do second round FFT on the response on wire
    c_resp = Array::dft_cc(c_resp, 1);

    // make ratio to the response and apply wire filter
    m_c_data[plane] = m_c_data[plane] / c_resp;

    // apply software filter on wire
    const std::vector<std::string> filter_names{"Wire_ind", "Wire_ind", "Wire_col"};
    Waveform::realseq_t wire_filter_wf;
    auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", filter_names[plane]);
    wire_filter_wf = ncr1->filter_waveform(m_c_data[plane].rows());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            float val = abs(m_c_data[plane](irow, icol));
            if (std::isnan(val)) {
                m_c_data[plane](irow, icol) = -0.0;
            }
            if (std::isinf(val)) {
                m_c_data[plane](irow, icol) = 0.0;
            }
            m_c_data[plane](irow, icol) *= wire_filter_wf.at(irow);
        }
    }

    // do the first round of inverse FFT on wire
    m_c_data[plane] = Array::idft_cc(m_c_data[plane], 1);

    // do the second round of inverse FFT on time
    m_r_data[plane] = Array::idft_cr(m_c_data[plane], 0);

    // do the shift in wire
    const int nrows = m_r_data[plane].rows();
    const int ncols = m_r_data[plane].cols();
    {
        Array::array_xxf arr1(m_wire_shift[plane], ncols);
        arr1 = m_r_data[plane].block(nrows - m_wire_shift[plane], 0, m_wire_shift[plane], ncols);
        Array::array_xxf arr2(nrows - m_wire_shift[plane], ncols);
        arr2 = m_r_data[plane].block(0, 0, nrows - m_wire_shift[plane], ncols);
        m_r_data[plane].block(0, 0, m_wire_shift[plane], ncols) = arr1;
        m_r_data[plane].block(m_wire_shift[plane], 0, nrows - m_wire_shift[plane], ncols) = arr2;
    }

    // do the shift in time
    int time_shift = (m_coarse_time_offset + m_intrinsic_time_offset) / m_period;
    if (time_shift > 0) {
        Array::array_xxf arr1(nrows, ncols - time_shift);
        arr1 = m_r_data[plane].block(0, 0, nrows, ncols - time_shift);
        Array::array_xxf arr2(nrows, time_shift);
        arr2 = m_r_data[plane].block(0, ncols - time_shift, nrows, time_shift);
        m_r_data[plane].block(0, 0, nrows, time_shift) = arr2;
        m_r_data[plane].block(0, time_shift, nrows, ncols - time_shift) = arr1;
    }
    m_c_data[plane] = Array::dft_rc(m_r_data[plane], 0);
}

void OmnibusSigProc::decon_2D_ROI_refine(int plane)
{
    // apply software filter on time

    const std::vector<std::string> filter_names{"Wiener_tight_U", "Wiener_tight_V", "Wiener_tight_W"};
    Waveform::realseq_t roi_hf_filter_wf;

    auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", filter_names[plane]);
    roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    restore_baseline(m_r_data[plane]);
}

void OmnibusSigProc::decon_2D_tightROI(int plane)
{
    // apply software filter on time

    Waveform::realseq_t roi_hf_filter_wf;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_U");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tight_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_V");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tight_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_W");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    restore_baseline(m_r_data[plane]);
}

// same as above but with "tight" -> "tighter" for ROI filterss.
void OmnibusSigProc::decon_2D_tighterROI(int plane)
{
    // apply software filter on time

    Waveform::realseq_t roi_hf_filter_wf;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_U");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tighter_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_V");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tighter_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_W");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    restore_baseline(m_r_data[plane]);
}

void OmnibusSigProc::decon_2D_looseROI(int plane)
{
    if (plane == 2) {
        return;  // don't filter colleciton
    }

    // apply software filter on time

    Waveform::realseq_t roi_hf_filter_wf;
    Waveform::realseq_t roi_hf_filter_wf1;
    Waveform::realseq_t roi_hf_filter_wf2;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_U");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        roi_hf_filter_wf1 = roi_hf_filter_wf;
        {
            auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_loose_lf");
            auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
            for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
                roi_hf_filter_wf.at(i) *= temp_filter.at(i);
            }
        }
        {
            auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tight_lf");
            auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
            for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
                roi_hf_filter_wf1.at(i) *= temp_filter.at(i);
            }
        }
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_V");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        roi_hf_filter_wf1 = roi_hf_filter_wf;
        {
            auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_loose_lf");
            auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
            for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
                roi_hf_filter_wf.at(i) *= temp_filter.at(i);
            }
        }
        {
            auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_tight_lf");
            auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
            for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
                roi_hf_filter_wf1.at(i) *= temp_filter.at(i);
            }
        }
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_W");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    const int n_lfn_nn = 2;
    const int n_bad_nn = plane ? 1 : 2;

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (auto och : m_channel_range[plane]) {
        const int irow = och.wire;

        roi_hf_filter_wf2 = roi_hf_filter_wf;
        if (masked_neighbors("bad", och, n_bad_nn) or masked_neighbors("lf_noisy", och, n_lfn_nn)) {
            roi_hf_filter_wf2 = roi_hf_filter_wf1;
        }

        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf2.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    restore_baseline(m_r_data[plane]);
}

// similar as decon_2D_looseROI() but without tightLF
void OmnibusSigProc::decon_2D_looseROI_debug_mode(int plane)
{
    // apply software filter on time
    if (plane == 2) {
        return;  // don't filter colleciton
    }

    Waveform::realseq_t roi_hf_filter_wf;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_U");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_loose_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_V");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
        auto ncr2 = Factory::find<IFilterWaveform>("LfFilter", "ROI_loose_lf");
        auto temp_filter = ncr2->filter_waveform(m_c_data[plane].cols());
        for (size_t i = 0; i != roi_hf_filter_wf.size(); i++) {
            roi_hf_filter_wf.at(i) *= temp_filter.at(i);
        }
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_tight_W");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    restore_baseline(m_r_data[plane]);
}

// return true if any channels w/in +/- nnn, inclusive, of the channel has the mask.
bool OmnibusSigProc::masked_neighbors(const std::string& cmname, OspChan& ochan, int nnn)
{
    // take care of boundary cases
    int lo_wire = ochan.wire - nnn;
    int lo_chan = ochan.channel - nnn;
    while (lo_wire < 0) {
        ++lo_wire;
        ++lo_chan;
    }
    const int nwires = m_nwires[ochan.plane];
    int hi_wire = ochan.wire + nnn;
    int hi_chan = ochan.channel + nnn;
    while (hi_wire >= nwires) {
        --hi_wire;
        --hi_chan;
    }
    if (hi_chan < lo_chan) {  // how?  bogus inputs?
        return false;
    }

    auto& cm = m_cmm[cmname];
    for (int och = lo_chan; och <= hi_chan; ++och) {
        if (cm.find(och) != cm.end()) {
            return true;
        }
    }
    return false;
}

void OmnibusSigProc::decon_2D_hits(int plane)
{
    // apply software filter on time

    Waveform::realseq_t roi_hf_filter_wf;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_wide_U");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_wide_V");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Wiener_wide_W");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    if (plane == 2) {
        restore_baseline(m_r_data[plane]);
    }
}

void OmnibusSigProc::decon_2D_charge(int plane)
{
    // apply software filter on time

    Waveform::realseq_t roi_hf_filter_wf;
    if (plane == 0) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Gaus_wide");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }
    else if (plane == 1) {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Gaus_wide");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }
    else {
        auto ncr1 = Factory::find<IFilterWaveform>("HfFilter", "Gaus_wide");
        roi_hf_filter_wf = ncr1->filter_waveform(m_c_data[plane].cols());
    }

    Array::array_xxc c_data_afterfilter(m_c_data[plane].rows(), m_c_data[plane].cols());
    for (int irow = 0; irow < m_c_data[plane].rows(); ++irow) {
        for (int icol = 0; icol < m_c_data[plane].cols(); ++icol) {
            c_data_afterfilter(irow, icol) = m_c_data[plane](irow, icol) * roi_hf_filter_wf.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    m_r_data[plane] = tm_r_data.block(m_pad_nwires[plane], 0, m_nwires[plane], m_nticks);
    if (plane == 2) {
        restore_baseline(m_r_data[plane]);
    }
}

bool OmnibusSigProc::operator()(const input_pointer& in, output_pointer& out)
{
    out = nullptr;
    if (!in) {
        log->debug("EOS at call={} anode={}",
                   m_count, m_anode->ident());
        ++m_count;
        return true;
    }
    const size_t ntraces = in->traces()->size();
    if (!ntraces) {
        out = std::make_shared<SimpleFrame>(in->ident(), in->time(), std::make_shared<ITrace::vector>(), in->tick());
        log->debug("call={} forwarding empty frame={} ",
                   m_count, out->ident());
        ++m_count;
        return true;
    }

    // Convert to OSP cmm indexed by OSB sequential channels, NOT WCT channel ID.
    m_cmm.clear();
    // double emap: name -> channel -> pair<int,int>
    for (auto cm : in->masks()) {
        const std::string name = cm.first;
        for (auto m : cm.second) {
            const int wct_channel_ident = m.first;
            const OspChan& och = m_channel_map[wct_channel_ident];
            if (och.plane < 0) {
                continue;  // in case user gives us multi apa frame
            }
            m_cmm[name][och.channel] = m.second;
        }
    }

    ITrace::vector* itraces = new ITrace::vector;  // will become shared_ptr.
    IFrame::trace_summary_t thresholds;
    IFrame::trace_list_t wiener_traces, gauss_traces, perframe_traces[3];
    // here are some trace lists for debug mode
    IFrame::trace_list_t tight_lf_traces, loose_lf_traces, cleanup_roi_traces, break_roi_loop1_traces,
        break_roi_loop2_traces, shrink_roi_traces, extend_roi_traces;
    IFrame::trace_list_t mp2_roi_traces, mp3_roi_traces;
    IFrame::trace_list_t decon_charge_traces;

    // initialize the overall response function ...
    init_overall_response(in);

    // create a class for ROIs ...
    ROI_formation roi_form(m_cmm, m_nwires[0], m_nwires[1], m_nwires[2], m_nticks, m_th_factor_ind, m_th_factor_col,
                           m_pad, m_asy, m_rebin, m_l_factor, m_l_max_th, m_l_factor1, m_l_short_length,
                           m_l_jump_one_bin);
    ROI_refinement roi_refine(
        m_cmm, m_nwires[0], m_nwires[1], m_nwires[2], m_r_th_factor, m_r_fake_signal_low_th, m_r_fake_signal_high_th,
        m_r_fake_signal_low_th_ind_factor, m_r_fake_signal_high_th_ind_factor, m_r_pad, m_r_break_roi_loop, m_r_th_peak,
        m_r_sep_peak, m_r_low_peak_sep_threshold_pre, m_r_max_npeaks, m_r_sigma, m_r_th_percent, m_isWrapped);  //

    const std::vector<float>* perplane_thresholds[3] = {&roi_form.get_uplane_rms(), &roi_form.get_vplane_rms(),
                                                        &roi_form.get_wplane_rms()};

    for (int iplane = 0; iplane != 3; ++iplane) {
        auto it = std::find(m_process_planes.begin(), m_process_planes.end(), iplane);
        if (it == m_process_planes.end()) continue;
        const std::vector<float>& perwire_rmses = *perplane_thresholds[iplane];

        // load data into EIGEN matrices ...
        load_data(in, iplane);  // load into a large matrix
        // initial decon ...
        decon_2D_init(iplane);  // decon in large matrix

        // Form tight ROIs
        if (iplane != 2) {  // induction wire planes
            if (m_use_roi_refinement) {
                decon_2D_tighterROI(iplane);
                Array::array_xxf r_data_tight = m_r_data[iplane];
                //      r_data_tight = m_r_data[plane];
                decon_2D_tightROI(iplane);
                roi_form.find_ROI_by_decon_itself(iplane, m_r_data[iplane], r_data_tight);
            }
        }
        else {  // collection wire planes
            decon_2D_tightROI(iplane);
            roi_form.find_ROI_by_decon_itself(iplane, m_r_data[iplane]);
        }

        // [wgu] save decon result after tight LF
        std::vector<double> dummy;
        if (m_use_roi_debug_mode and m_use_roi_refinement) save_data(*itraces, tight_lf_traces, iplane, perwire_rmses, dummy);

        // Form loose ROIs
        if (iplane != 2) {
            // [wgu] save decon result after loose LF
            if (m_use_roi_debug_mode) {
                decon_2D_looseROI_debug_mode(iplane);
                save_data(*itraces, loose_lf_traces, iplane, perwire_rmses, dummy);
            }

            if (m_use_roi_refinement) {
                decon_2D_looseROI(iplane);
                roi_form.find_ROI_loose(iplane, m_r_data[iplane]);
                decon_2D_ROI_refine(iplane);
            }
        }

        // [wgu] collection plane does not need loose LF
        // but save something to be consistent
        if (m_use_roi_debug_mode and iplane == 2) save_data(*itraces, loose_lf_traces, iplane, perwire_rmses, dummy);

        // Refine ROIs
        if (m_use_roi_refinement) roi_refine.load_data(iplane, m_r_data[iplane], roi_form);
        else {
          m_c_data[iplane].resize(0, 0);  // clear memory
          m_r_data[iplane].resize(0, 0);  // clear memory
        }
    }

    if (m_use_roi_refinement) {
    for (int iplane = 0; iplane != 3; ++iplane) {
        auto it = std::find(m_process_planes.begin(), m_process_planes.end(), iplane);
        if (it == m_process_planes.end()) continue;

        // roi_refine.refine_data(iplane, roi_form);

        roi_refine.CleanUpROIs(iplane);
        roi_refine.generate_merge_ROIs(iplane);

        if (m_use_roi_debug_mode) {
            save_roi(*itraces, cleanup_roi_traces, iplane, roi_refine.get_rois_by_plane(iplane));
        }

        if (m_use_multi_plane_protection) {
            roi_refine.MultiPlaneProtection(iplane, m_anode, m_roi_ch_ch_ident, roi_form, 1000, m_anode->ident() % 2);
            save_mproi(*itraces, mp3_roi_traces, iplane, roi_refine.get_mp3_rois());
            roi_refine.MultiPlaneROI(iplane, m_anode, m_roi_ch_ch_ident, roi_form, 1000, m_anode->ident() % 2);
            save_mproi(*itraces, mp2_roi_traces, iplane, roi_refine.get_mp2_rois());
        }
    }

    for (int iplane = 0; iplane != 3; ++iplane) {
        auto it = std::find(m_process_planes.begin(), m_process_planes.end(), iplane);
        if (it == m_process_planes.end()) continue;

        const std::vector<float>& perwire_rmses = *perplane_thresholds[iplane];

        for (int qx = 0; qx != m_r_break_roi_loop; qx++) {
            roi_refine.BreakROIs(iplane, roi_form);
            roi_refine.CheckROIs(iplane, roi_form);
            roi_refine.CleanUpROIs(iplane);
            if (m_use_roi_debug_mode) {
                if (qx == 0) save_roi(*itraces, break_roi_loop1_traces, iplane, roi_refine.get_rois_by_plane(iplane));
                if (qx == 1) save_roi(*itraces, break_roi_loop2_traces, iplane, roi_refine.get_rois_by_plane(iplane));
            }
        }

        roi_refine.ShrinkROIs(iplane, roi_form);
        roi_refine.CheckROIs(iplane, roi_form);
        roi_refine.CleanUpROIs(iplane);
        if (m_use_roi_debug_mode) {
            save_roi(*itraces, shrink_roi_traces, iplane, roi_refine.get_rois_by_plane(iplane));
        }

        if (iplane == 2) {
            roi_refine.CleanUpCollectionROIs();
        }
        else {
            roi_refine.CleanUpInductionROIs(iplane);
        }
        roi_refine.ExtendROIs(iplane);

        if (m_use_roi_debug_mode) {
            save_ext_roi(*itraces, extend_roi_traces, iplane, roi_refine.get_rois_by_plane(iplane));
        }

        // merge results ...
        decon_2D_hits(iplane);
        roi_refine.apply_roi(iplane, m_r_data[iplane]);
        // roi_form.apply_roi(iplane, m_r_data[plane],1);
        save_data(*itraces, perframe_traces[iplane], iplane, perwire_rmses, thresholds);
        wiener_traces.insert(wiener_traces.end(), perframe_traces[iplane].begin(), perframe_traces[iplane].end());

        decon_2D_charge(iplane);
        std::vector<double> dummy_thresholds;
        if (m_use_roi_debug_mode) {
            save_data(*itraces, decon_charge_traces, iplane, perwire_rmses, thresholds);
        }
        roi_refine.apply_roi(iplane, m_r_data[iplane]);
        // roi_form.apply_roi(iplane, m_r_data[plane],1);
        save_data(*itraces, gauss_traces, iplane, perwire_rmses, dummy_thresholds);

        m_c_data[iplane].resize(0, 0);  // clear memory
        m_r_data[iplane].resize(0, 0);  // clear memory
    }
    }

    SimpleFrame* sframe = new SimpleFrame(in->ident(), in->time(), ITrace::shared_vector(itraces), in->tick(), m_cmm);
    sframe->tag_frame(m_frame_tag);

    // this assumes save_data produces itraces in OSP channel order
    // std::vector<float> perplane_thresholds[3] = {
    //   roi_form.get_uplane_rms(),
    //   roi_form.get_vplane_rms(),
    //   roi_form.get_wplane_rms()
    // };

    // IFrame::trace_summary_t threshold;
    // for (int iplane=0; iplane<3; ++iplane) {
    //   for (float val : perplane_thresholds[iplane]) {
    //     threshold.push_back((double)val);
    //   }
    // }

    if (m_use_roi_refinement) {
        sframe->tag_traces(m_wiener_tag, wiener_traces);
        sframe->tag_traces(m_wiener_threshold_tag, wiener_traces, thresholds);
        sframe->tag_traces(m_gauss_tag, gauss_traces);
    }

    if (m_use_roi_debug_mode) {
        sframe->tag_traces(m_loose_lf_tag, loose_lf_traces);
        if (m_use_roi_refinement) {
            sframe->tag_traces(m_decon_charge_tag, decon_charge_traces);
            sframe->tag_traces(m_tight_lf_tag, tight_lf_traces);
            sframe->tag_traces(m_cleanup_roi_tag, cleanup_roi_traces);
            sframe->tag_traces(m_break_roi_loop1_tag, break_roi_loop1_traces);
            sframe->tag_traces(m_break_roi_loop2_tag, break_roi_loop2_traces);
            sframe->tag_traces(m_shrink_roi_tag, shrink_roi_traces);
            sframe->tag_traces(m_extend_roi_tag, extend_roi_traces);
        }
    }

    if (m_use_multi_plane_protection) {
        sframe->tag_traces(m_mp3_roi_tag, mp3_roi_traces);
        sframe->tag_traces(m_mp2_roi_tag, mp2_roi_traces);
    }

    log->debug("call={} produce {} "
               "traces: {} {}, {} {}, {} {}, frame tag: {}",
               m_count,
               itraces->size(),
               wiener_traces.size(), m_wiener_tag,
               decon_charge_traces.size(), m_decon_charge_tag,
               gauss_traces.size(), m_gauss_tag,
               m_frame_tag);

    out = IFrame::pointer(sframe);
    ++m_count;

    return true;
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
