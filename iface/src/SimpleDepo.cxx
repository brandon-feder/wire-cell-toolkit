#include "WireCellIface/SimpleDepo.h"

using namespace WireCell;

SimpleDepo::SimpleDepo(double t, const WireCell::Point& pos, double charge, IDepo::pointer prior, double extent_long,
                       double extent_tran, int id, int pdg, double energy)

  : m_time(t)
  , m_pos(pos)
  , m_id(id)
  , m_pdg(pdg)
  , m_charge(charge)
  , m_energy(energy)
  , m_prior(prior)
  , m_long(extent_long)
  , m_tran(extent_tran)
{
}

const WireCell::Point& SimpleDepo::pos() const { return m_pos; }
double SimpleDepo::time() const { return m_time; }
double SimpleDepo::charge() const { return m_charge; }
double SimpleDepo::energy() const { return m_energy; }
WireCell::IDepo::pointer SimpleDepo::prior() const { return m_prior; }
int SimpleDepo::id() const { return m_id; }
int SimpleDepo::pdg() const { return m_pdg; }
double SimpleDepo::extent_long() const { return m_long; }
double SimpleDepo::extent_tran() const { return m_tran; }
