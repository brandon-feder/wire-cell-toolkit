#include "WireCellSigProc/PerChannelResponse.h"

#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/String.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(PerChannelResponse, WireCell::SigProc::PerChannelResponse, WireCell::IChannelResponse,
                 WireCell::IConfigurable)

using namespace WireCell;

SigProc::PerChannelResponse::PerChannelResponse(const char* filename)
  : m_filename(filename)
{
}

SigProc::PerChannelResponse::~PerChannelResponse() {}

WireCell::Configuration SigProc::PerChannelResponse::default_configuration() const
{
    Configuration cfg;
    cfg["filename"] = m_filename;
    return cfg;
}

void SigProc::PerChannelResponse::configure(const WireCell::Configuration& cfg)
{
    m_filename = get(cfg, "filename", m_filename);
    if (m_filename.empty()) {
        THROW(ValueError() << errmsg{"must supply a PerChannelResponse filename"});
    }

    auto top = Persist::load(m_filename);
    const double tick = top["tick"].asFloat();
    const double t0 = top["t0"].asFloat();
    auto jchannels = top["channels"];
    if (jchannels.isNull()) {
        THROW(ValueError() << errmsg{"no channels given in file " + m_filename});
    }

    for (auto jchresp : jchannels) {
        const int ch = jchresp[0].asInt();
        auto jresp = jchresp[1];
        const int nsamp = jresp.size();
        if (nsamp == 0) {
            THROW(ValueError() << errmsg{"zero length response in file " + m_filename});
        }
        Waveform::realseq_t resp(nsamp, 0);
        for (int ind = 0; ind < nsamp; ++ind) {
            resp[ind] = jresp[ind].asFloat();
        }
        m_cr[ch] = resp;
        if (!m_bins.nbins()) {  // first time
            m_bins = Binning(nsamp, t0, t0 + nsamp * tick);
        }
    }
}

const Waveform::realseq_t& SigProc::PerChannelResponse::channel_response(int channel_ident) const
{
    const auto& it = m_cr.find(channel_ident);
    if (it == m_cr.end()) {
        THROW(KeyError() << errmsg{String::format("no response for channel %d", channel_ident)});
    }
    return it->second;
}

Binning SigProc::PerChannelResponse::channel_response_binning() const { return m_bins; }
