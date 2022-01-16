#include "WireCellIface/SimpleChannel.h"

using namespace WireCell;

SimpleChannel::SimpleChannel(int ident, int index, const IWire::vector& wires)
  : m_ident(ident)
  , m_index(index)
  , m_wires(wires.begin(), wires.end())
{
    std::sort(m_wires.begin(), m_wires.end(), IWireCompareSegment());
}

SimpleChannel::~SimpleChannel() {}

int SimpleChannel::ident() const { return m_ident; }
int SimpleChannel::index() const { return m_index; }
const IWire::vector& SimpleChannel::wires() const { return m_wires; }

// Personal interface for creator
//
void SimpleChannel::add(const IWire::pointer& wire)
{
    m_wires.push_back(wire);
    std::sort(m_wires.begin(), m_wires.end(), IWireCompareSegment());
}
void SimpleChannel::set_index(int ind) { m_index = ind; }
