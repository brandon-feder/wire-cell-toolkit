#include "WireCellGen/Diffusion.h"
using namespace WireCell;

Diffusion::Diffusion(IDepo::pointer depo, int nlong, int ntrans, double lmin, double tmin, double lmax, double tmax)
  : m_depo(depo)
  , array(boost::extents[nlong][ntrans])
  , lmin(lmin)
  , tmin(tmin)
  , lmax(lmax)
  , tmax(tmax)
{
    lbin = (lmax - lmin) / nlong;
    tbin = (tmax - tmin) / ntrans;
}

Diffusion::Diffusion(const Diffusion& other)
  : m_depo(other.m_depo)
  , array(other.array)
  , lmin(other.lmin)
  , tmin(other.tmin)
  , lmax(other.lmax)
  , tmax(other.tmax)
{
}

Diffusion& Diffusion::operator=(const Diffusion& other)
{
    m_depo = other.m_depo;
    array.resize(boost::extents[other.lsize()][other.tsize()]);
    array = other.array;
    lmin = other.lmin;
    tmin = other.tmin;
    lmax = other.lmax;
    tmax = other.tmax;
    // isn't C++ fun?
    return *this;
}

Diffusion::~Diffusion() {}

IDepo::pointer Diffusion::depo() const { return m_depo; }
int Diffusion::lsize() const { return array.shape()[0]; }
int Diffusion::tsize() const { return array.shape()[1]; }

double Diffusion::get(int lind, int tind) const { return array[lind][tind]; }
double Diffusion::set(int lind, int tind, double value)
{
    array[lind][tind] = value;
    return value;
}

double Diffusion::lpos(int ind, double offset) const { return lmin + (ind + offset) * lbin; }

double Diffusion::tpos(int ind, double offset) const { return tmin + (ind + offset) * tbin; }
