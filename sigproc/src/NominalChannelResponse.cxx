#include "WireCellSigProc/NominalChannelResponse.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Exceptions.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(NominalChannelResponse, WireCell::SigProc::NominalChannelResponse, WireCell::IChannelResponse,
                 WireCell::IConfigurable)

using namespace WireCell;
SigProc::NominalChannelResponse::NominalChannelResponse(double gain, double shaping, const Binning& binning)
  : m_gain(gain)
  , m_shaping(shaping)
  , m_bins(binning)
{
}

SigProc::NominalChannelResponse::~NominalChannelResponse() {}

WireCell::Configuration SigProc::NominalChannelResponse::default_configuration() const
{
    Configuration cfg;
    cfg["gain"] = m_gain;
    cfg["shaping"] = m_shaping;
    cfg["nbins"] = m_bins.nbins();
    cfg["tmin"] = m_bins.min();
    cfg["tmax"] = m_bins.max();
    return cfg;
}

void SigProc::NominalChannelResponse::configure(const WireCell::Configuration& cfg)
{
    m_gain = get(cfg, "gain", m_gain);
    m_shaping = get(cfg, "shaping", m_shaping);
    int nbins = get(cfg, "nbins", m_bins.nbins());
    double tmin = get(cfg, "tmin", m_bins.min());
    double tmax = get(cfg, "tmax", m_bins.max());
    Response::ColdElec ce(m_gain, m_shaping);
    m_bins = Binning(nbins, tmin, tmax);
    m_cr = ce.generate(m_bins);
    if (m_cr.empty()) {
        THROW(ValueError() << errmsg{"Failed to generate any nominal channel response"});
    }
}

const Waveform::realseq_t& SigProc::NominalChannelResponse::channel_response(int channel_ident) const { return m_cr; }

Binning SigProc::NominalChannelResponse::channel_response_binning() const { return m_bins; }
