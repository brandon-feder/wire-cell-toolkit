#include "WireCellGen/ZSEndedTrace.h"

using namespace std;
using namespace WireCell;

ZSEndedTrace::ZSEndedTrace(int chid, int nbins)
  : m_chid(chid)
  , m_nbins(nbins)
{
}

void ZSEndedTrace::operator()(int bin, float charge)
{
    bin = min(bin, m_nbins - 1);
    m_chqmap[bin] += charge;
    m_charge.clear();  // invalidate;
}

int ZSEndedTrace::channel() const { return m_chid; }

int ZSEndedTrace::tbin() const { return m_chqmap.begin()->first; }

const ZSEndedTrace::ChargeSequence& ZSEndedTrace::charge() const
{
    if (!m_charge.size()) {
        int first_bin = m_chqmap.begin()->first;
        int last_bin = m_chqmap.rbegin()->first;
        m_charge.resize(last_bin - first_bin + 1, 0);
        for (auto mit : m_chqmap) {
            m_charge[mit.first - first_bin] = mit.second;
        }
    }
    return m_charge;
}
