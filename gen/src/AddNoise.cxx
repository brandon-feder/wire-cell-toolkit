#include "WireCellGen/AddNoise.h"

#include "WireCellIface/SimpleTrace.h"
#include "WireCellIface/SimpleFrame.h"

#include "WireCellUtil/Persist.h"
#include "WireCellUtil/NamedFactory.h"

#include "Noise.h"

#include <iostream>

WIRECELL_FACTORY(AddNoise, WireCell::Gen::AddNoise,
                 WireCell::INamed,
                 WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::AddNoise::AddNoise(const std::string& model, const std::string& rng)
  : Aux::Logger("AddNoise", "gen")
  , m_model_tn(model)
  , m_rng_tn(rng)
  , m_nsamples(9600)
  , m_rep_percent(0.02)  // replace 2% at a time
{
}

Gen::AddNoise::~AddNoise() {}

WireCell::Configuration Gen::AddNoise::default_configuration() const
{
    Configuration cfg;

    // fixme: maybe add some tag support?

    cfg["model"] = m_model_tn;
    cfg["rng"] = m_rng_tn;
    cfg["nsamples"] = m_nsamples;
    cfg["replacement_percentage"] = m_rep_percent;
    return cfg;
}

void Gen::AddNoise::configure(const WireCell::Configuration& cfg)
{
    m_rng_tn = get(cfg, "rng", m_rng_tn);
    m_rng = Factory::find_tn<IRandom>(m_rng_tn);
    m_model_tn = get(cfg, "model", m_model_tn);
    m_model = Factory::find_tn<IChannelSpectrum>(m_model_tn);
    m_nsamples = get<int>(cfg, "nsamples", m_nsamples);
    m_rep_percent = get<double>(cfg, "replacement_percentage", m_rep_percent);

    log->debug("using IRandom: \"{}\", IChannelSpectrum: \"{}\"", m_rng_tn, m_model_tn);
}

bool Gen::AddNoise::operator()(const input_pointer& inframe, output_pointer& outframe)
{
    if (!inframe) {
        outframe = nullptr;
        log->debug("EOS at call={}", m_count);
        ++m_count;
        return true;
    }

    ITrace::vector outtraces;
    for (const auto& intrace : *inframe->traces()) {
        int chid = intrace->channel();
        const auto& spec = (*m_model)(chid);
        Waveform::realseq_t wave = Gen::Noise::generate_waveform(spec, m_rng, m_rep_percent);

        wave.resize(m_nsamples, 0);
        Waveform::increase(wave, intrace->charge());
        auto trace = make_shared<SimpleTrace>(chid, intrace->tbin(), wave);
        outtraces.push_back(trace);
    }
    outframe = make_shared<SimpleFrame>(inframe->ident(), inframe->time(), outtraces, inframe->tick());
    log->debug("call={} frame={} {} traces",
               m_count, inframe->ident(), outtraces.size());
    ++m_count;
    return true;
}
