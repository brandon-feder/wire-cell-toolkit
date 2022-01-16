#include "WireCellGen/RecombinationModels.h"

#include "WireCellUtil/NamedFactory.h"

#include <cmath>

WIRECELL_FACTORY(MipRecombination, WireCell::Gen::MipRecombination, WireCell::IRecombinationModel,
                 WireCell::IConfigurable)
WIRECELL_FACTORY(BirksRecombination, WireCell::Gen::MipRecombination, WireCell::IRecombinationModel,
                 WireCell::IConfigurable)
WIRECELL_FACTORY(BoxRecombination, WireCell::Gen::MipRecombination, WireCell::IRecombinationModel,
                 WireCell::IConfigurable)

using namespace WireCell;

/*
  MIP recombination model
*/
Gen::MipRecombination::MipRecombination(double Rmip, double Wi)
  : m_rmip(Rmip)
  , m_wi(Wi)
{
}
Gen::MipRecombination::~MipRecombination() {}
double Gen::MipRecombination::operator()(double dE, double dX) { return m_rmip * dE / m_wi; }
void Gen::MipRecombination::configure(const WireCell::Configuration& config)
{
    m_rmip = get(config, "Rmip", m_rmip);
    m_wi = get(config, "Wi", m_wi);
}
WireCell::Configuration Gen::MipRecombination::default_configuration() const
{
    Configuration cfg;
    cfg["Rmip"] = m_rmip;
    cfg["Wi"] = m_wi;
    return cfg;
}

/*
  Birks Recombination Model
 */
Gen::BirksRecombination::BirksRecombination(double Efield, double A3t, double k3t, double rho, double Wi)
  : m_a3t(A3t)
  , m_k3t(k3t)
  , m_efield(Efield)
  , m_rho(rho)
  , m_wi(Wi)
{
}
Gen::BirksRecombination::~BirksRecombination() {}
double Gen::BirksRecombination::operator()(double dE, double dX)
{
    const double R = m_a3t / (1 + (dE / dX) * m_k3t / (m_efield * m_rho));
    return R * dE / m_wi;
}
void Gen::BirksRecombination::configure(const WireCell::Configuration& config)
{
    m_a3t = get(config, "A3t", m_a3t);
    m_k3t = get(config, "k3t", m_k3t);
    m_efield = get(config, "Efield", m_efield);
    m_rho = get(config, "rho", m_rho);
    m_wi = get(config, "Wi", m_wi);
}
WireCell::Configuration Gen::BirksRecombination::default_configuration() const
{
    Configuration cfg;
    cfg["A3t"] = m_a3t;
    cfg["k3t"] = m_k3t;
    cfg["Efield"] = m_efield;
    cfg["rho"] = m_rho;
    cfg["Wi"] = m_wi;
    return cfg;
}

/*
  Modified Box Model
*/
Gen::BoxRecombination::BoxRecombination(double Efield, double A, double B, double rho, double Wi)
  : m_efield(Efield)
  , m_a(A)
  , m_b(B)
  , m_rho(rho)
  , m_wi(Wi)
{
}
Gen::BoxRecombination::~BoxRecombination() {}
double Gen::BoxRecombination::operator()(double dE, double dX)
{
    const double tmp = (dE / dX) * m_b / (m_efield * m_rho);
    const double R = std::log(m_a + tmp) / tmp;
    return R * dE / m_wi;
}
void Gen::BoxRecombination::configure(const WireCell::Configuration& config)
{
    m_efield = get(config, "Efield", m_efield);
    m_a = get(config, "A", m_a);
    m_b = get(config, "B", m_b);
    m_rho = get(config, "rho", m_rho);
    m_wi = get(config, "Wi", m_wi);
}
WireCell::Configuration Gen::BoxRecombination::default_configuration() const
{
    Configuration cfg;
    cfg["Efield"] = m_efield;
    cfg["A"] = m_a;
    cfg["B"] = m_b;
    cfg["rho"] = m_rho;
    cfg["Wi"] = m_wi;
    return cfg;
}
