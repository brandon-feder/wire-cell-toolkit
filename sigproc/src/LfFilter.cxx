#include "WireCellSigProc/LfFilter.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(LfFilter, WireCell::SigProc::LfFilter, WireCell::IFilterWaveform, WireCell::IConfigurable)

using namespace WireCell;

SigProc::LfFilter::LfFilter(double max_freq, double tau)
  : m_max_freq(max_freq)
  , m_tau(tau)
{
}

SigProc::LfFilter::~LfFilter() {}

WireCell::Configuration SigProc::LfFilter::default_configuration() const
{
    Configuration cfg;
    cfg["tau"] = m_tau;
    cfg["max_freq"] = m_max_freq;
    return cfg;
}

void SigProc::LfFilter::configure(const WireCell::Configuration& cfg)
{
    m_tau = get(cfg, "tau", m_tau);
    m_max_freq = get(cfg, "max_freq", m_max_freq);
}

const Waveform::realseq_t SigProc::LfFilter::filter_waveform(int nbins) const
{
    Waveform::realseq_t m_wfs(nbins);

    Response::LfFilter lf_filter(m_tau);

    for (size_t i = 0; i != m_wfs.size(); i++) {
        double freq = i * 1.0 / int(m_wfs.size()) * 2 * m_max_freq;
        if (freq > m_max_freq) freq = freq - 2 * m_max_freq;
        m_wfs.at(i) = lf_filter(fabs(freq));
    }
    // std::cout << m_wfs.size() << std::endl;

    return m_wfs;
}
