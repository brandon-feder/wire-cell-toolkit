#include "WireCellGen/NoiseSource.h"

#include "WireCellIface/SimpleTrace.h"
#include "WireCellIface/SimpleFrame.h"

#include "WireCellUtil/Persist.h"
#include "WireCellUtil/NamedFactory.h"

#include "Noise.h"

#include <iostream>

WIRECELL_FACTORY(NoiseSource, WireCell::Gen::NoiseSource, WireCell::IFrameSource, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::NoiseSource::NoiseSource(const std::string& model, const std::string& anode, const std::string& rng)
  : m_time(0.0 * units::ns)
  , m_stop(1.0 * units::ms)
  , m_readout(5.0 * units::ms)
  , m_tick(0.5 * units::us)
  , m_frame_count(0)
  , m_anode_tn(anode)
  , m_model_tn(model)
  , m_rng_tn(rng)
  , m_nsamples(9600)
  , m_rep_percent(0.02)  // replace 2% at a time
  , m_eos(false)
{
    // initialize the random number ...
    // auto& spec = (*m_model)(0);

    // end initialization ..
}

Gen::NoiseSource::~NoiseSource() {}

WireCell::Configuration Gen::NoiseSource::default_configuration() const
{
    Configuration cfg;
    cfg["start_time"] = m_time;
    cfg["stop_time"] = m_stop;
    cfg["readout_time"] = m_readout;
    cfg["sample_period"] = m_tick;
    cfg["first_frame_number"] = m_frame_count;

    cfg["anode"] = m_anode_tn;
    cfg["model"] = m_model_tn;
    cfg["rng"] = m_rng_tn;
    cfg["nsamples"] = m_nsamples;
    cfg["replacement_percentage"] = m_rep_percent;
    return cfg;
}

void Gen::NoiseSource::configure(const WireCell::Configuration& cfg)
{
    m_rng_tn = get(cfg, "rng", m_rng_tn);
    m_rng = Factory::find_tn<IRandom>(m_rng_tn);
    if (!m_rng) {
        THROW(KeyError() << errmsg{"failed to get IRandom: " + m_rng_tn});
    }

    m_anode_tn = get(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);
    if (!m_anode) {
        THROW(KeyError() << errmsg{"failed to get IAnodePlane: " + m_anode_tn});
    }

    m_model_tn = get(cfg, "model", m_model_tn);
    m_model = Factory::find_tn<IChannelSpectrum>(m_model_tn);
    if (!m_model) {
        THROW(KeyError() << errmsg{"failed to get IChannelSpectrum: " + m_model_tn});
    }

    m_readout = get<double>(cfg, "readout_time", m_readout);
    m_time = get<double>(cfg, "start_time", m_time);
    m_stop = get<double>(cfg, "stop_time", m_stop);
    m_tick = get<double>(cfg, "sample_period", m_tick);
    m_frame_count = get<int>(cfg, "first_frame_number", m_frame_count);
    m_nsamples = get<int>(cfg, "m_nsamples", m_nsamples);
    m_rep_percent = get<double>(cfg, "replacement_percentage", m_rep_percent);

    cerr << "Gen::NoiseSource: using IRandom: \"" << m_rng_tn << "\""
         << " IAnodePlane: \"" << m_anode_tn << "\""
         << " IChannelSpectrum: \"" << m_model_tn << "\""
         << " readout time: " << m_readout / units::us << "us\n";
}

bool Gen::NoiseSource::operator()(IFrame::pointer& frame)
{
    if (m_eos) {  // This source does not restart.
        return false;
    }

    if (m_time >= m_stop) {
        frame = nullptr;
        m_eos = true;
        return true;
    }
    ITrace::vector traces;
    const int tbin = 0;
    int nsamples = 0;
    for (auto chid : m_anode->channels()) {
        const auto& spec = (*m_model)(chid);

        Waveform::realseq_t noise = Gen::Noise::generate_waveform(spec, m_rng, m_rep_percent);
        //	std::cout << noise.size() << " " << nsamples << std::endl;
        noise.resize(m_nsamples, 0);
        auto trace = make_shared<SimpleTrace>(chid, tbin, noise);
        traces.push_back(trace);
        nsamples += noise.size();
    }
    cerr << "Gen::NoiseSource: made " << traces.size() << " traces, " << nsamples << " samples\n";
    frame = make_shared<SimpleFrame>(m_frame_count, m_time, traces, m_tick);
    m_time += m_readout;
    ++m_frame_count;
    return true;
}
