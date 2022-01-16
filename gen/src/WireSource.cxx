#include "WireCellGen/WireSource.h"
#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(WireSource, WireCell::WireSource, WireCell::IWireSource, WireCell::IConfigurable)

using namespace WireCell;

WireSource::WireSource()
  : m_params(new WireParams)
  , m_wiregen()
{
}

WireSource::~WireSource() {}

Configuration WireSource::default_configuration() const { return m_params->default_configuration(); }
void WireSource::configure(const Configuration& cfg) { m_params->configure(cfg); }

bool WireSource::operator()(output_pointer& wires) { return m_wiregen(m_params, wires); }
