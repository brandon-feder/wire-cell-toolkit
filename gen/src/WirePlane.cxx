#include "WireCellGen/WirePlane.h"

using namespace WireCell;

Gen::WirePlane::WirePlane(int ident, Pimpos* pimpos, const IWire::vector& wires, const IChannel::vector& channels)
  : m_ident(ident)
  , m_pimpos(pimpos)
  , m_wires(wires)
  , m_channels(channels)
{
}

Gen::WirePlane::~WirePlane() {}
