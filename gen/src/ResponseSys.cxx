#include "WireCellGen/ResponseSys.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Response.h"

WIRECELL_FACTORY(ResponseSys, WireCell::Gen::ResponseSys, WireCell::IWaveform, WireCell::IConfigurable)

using namespace WireCell;

Gen::ResponseSys::ResponseSys(int nticks, double start, double tick, double magnitude, double time_smear, double offset)
{
    m_cfg["nticks"] = tick;
    m_cfg["start"] = start;
    m_cfg["tick"] = tick;
    m_cfg["magnitude"] = magnitude;
    m_cfg["time_smear"] = time_smear;
    m_cfg["offset"] = offset;
}
WireCell::Configuration Gen::ResponseSys::default_configuration() const { return m_cfg; }
void Gen::ResponseSys::configure(const WireCell::Configuration& cfg)
{
    m_cfg = cfg;

    const double tick = waveform_period();
    const double offset = m_cfg["offset"].asDouble();
    const double sigma = m_cfg["time_smear"].asDouble();
    // Sys is a Gaussian function
    m_sysresp = new Response::SysResp(tick, m_cfg["magnitude"].asDouble(), sigma, offset);

    const int nbins = m_cfg["nticks"].asInt();
    const double start = waveform_start();
    Binning tbins(nbins, start, nbins * tick + start);
    m_wave = m_sysresp->generate(tbins);
}

double Gen::ResponseSys::waveform_start() const { return m_cfg["start"].asDouble(); }

double Gen::ResponseSys::waveform_period() const { return m_cfg["tick"].asDouble(); }

const IWaveform::sequence_type& Gen::ResponseSys::waveform_samples() const { return m_wave; }

IWaveform::sequence_type Gen::ResponseSys::waveform_samples(const WireCell::Binning& tbins) const
{
    sequence_type rebinned_wave = m_sysresp->generate(tbins);
    return rebinned_wave;
}
