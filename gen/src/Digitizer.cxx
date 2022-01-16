#include "WireCellGen/Digitizer.h"

#include "WireCellIface/IWireSelectors.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include "WireCellAux/FrameTools.h"

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/NamedFactory.h"
#include <sstream>

WIRECELL_FACTORY(Digitizer, WireCell::Gen::Digitizer,
                 WireCell::INamed,
                 WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::Digitizer::Digitizer(const std::string& anode,
                          int resolution, double gain,
                          std::vector<double> fullscale,
                          std::vector<double> baselines)
  : Aux::Logger("Digitizer", "gen")
  , m_anode_tn(anode)
  , m_resolution(resolution)
  , m_gain(gain)
  , m_fullscale(fullscale)
  , m_baselines(baselines)
  , m_frame_tag("")
{
}

Gen::Digitizer::~Digitizer() {}

WireCell::Configuration Gen::Digitizer::default_configuration() const
{
    Configuration cfg;
    put(cfg, "anode", m_anode_tn);

    put(cfg, "resolution", m_resolution);
    put(cfg, "gain", m_gain);
    Configuration fs(Json::arrayValue);  // fixme: sure would be nice if we had some templated sugar for this
    for (int ind = 0; ind < 2; ++ind) {
        fs[ind] = m_fullscale[ind];
    }
    cfg["fullscale"] = fs;

    Configuration bl(Json::arrayValue);
    for (int ind = 0; ind < 3; ++ind) {
        bl[ind] = m_baselines[ind];
    }
    cfg["baselines"] = bl;

    cfg["frame_tag"] = m_frame_tag;
    return cfg;
}

void Gen::Digitizer::configure(const Configuration& cfg)
{
    m_anode_tn = get<string>(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);

    m_resolution = get(cfg, "resolution", m_resolution);
    m_gain = get(cfg, "gain", m_gain);
    m_fullscale = get(cfg, "fullscale", m_fullscale);
    m_baselines = get(cfg, "baselines", m_baselines);
    m_frame_tag = get(cfg, "frame_tag", m_frame_tag);

    std::stringstream ss;
    ss << "tag=\"" << m_frame_tag << "\", "
       << "resolution=" << m_resolution << " bits, "
       << "maxvalue=" << (1 << m_resolution) << " counts, "
       << "gain=" << m_gain << ", "
       << "fullscale=[" << m_fullscale[0] / units::mV << "," << m_fullscale[1] / units::mV << "] mV, "
       << "baselines=[" << m_baselines[0] / units::mV << "," << m_baselines[1] / units::mV << ","
       << m_baselines[2] / units::mV << "] mV";
    log->debug(ss.str());
}

double Gen::Digitizer::digitize(double voltage)
{
    const int adcmaxval = (1 << m_resolution) - 1;

    if (voltage <= m_fullscale[0]) {
        return 0;
    }
    if (voltage >= m_fullscale[1]) {
        return adcmaxval;
    }
    const double relvoltage = (voltage - m_fullscale[0]) / (m_fullscale[1] - m_fullscale[0]);
    return relvoltage * adcmaxval;
}

bool Gen::Digitizer::operator()(const input_pointer& vframe, output_pointer& adcframe)
{
    if (!vframe) {  // EOS
        log->debug("see EOS at call={}", m_count);
        adcframe = nullptr;
        ++m_count;
        return true;
    }

    // fixme: maybe make this honor a tag
    auto vtraces = Aux::untagged_traces(vframe);
    if (vtraces.empty()) {
        log->error("no traces in input frame {} at call={}",
                   vframe->ident(), m_count);
        ++m_count;
        return false;
    }

    // Get extent in channel and tbin
    auto channels = Aux::channels(vtraces);
    std::sort(channels.begin(), channels.end());
    auto chbeg = channels.begin();
    auto chend = std::unique(chbeg, channels.end());
    auto tbinmm = Aux::tbin_range(vtraces);

    const size_t ncols = tbinmm.second - tbinmm.first;
    const size_t nrows = std::distance(chbeg, chend);

    // make a dense array working space.  a row is one trace.  a
    // column is one tick.
    Array::array_xxf arr = Array::array_xxf::Zero(nrows, ncols);
    Aux::fill(arr, vtraces, channels.begin(), chend, tbinmm.first);

    ITrace::vector adctraces(nrows);

    for (size_t irow = 0; irow < nrows; ++irow) {
        int ch = channels[irow];
        WirePlaneId wpid = m_anode->resolve(ch);
        if (!wpid.valid()) {
            log->warn("got invalid WPID for channel {}: {}, skipping", ch, wpid);
            continue;
        }
        const float baseline = m_baselines[wpid.index()];

        ITrace::ChargeSequence adcwave(ncols);
        for (size_t icol = 0; icol < ncols; ++icol) {
            double voltage = m_gain * arr(irow, icol) + baseline;
            const float adcf = digitize(voltage);
            adcwave[icol] = adcf;
        }
        adctraces[irow] = make_shared<SimpleTrace>(ch, tbinmm.first, adcwave);
    }
    auto sframe = make_shared<SimpleFrame>(vframe->ident(), vframe->time(), adctraces, vframe->tick(), vframe->masks());
    if (!m_frame_tag.empty()) {
        sframe->tag_frame(m_frame_tag);
    }
    adcframe = sframe;

    log->debug("call={} traces={} frame={} outtag=\"{}\"",
               m_count,
               adctraces.size(), vframe->ident(), m_frame_tag);
    ++m_count;
    return true;
}
