#include "WireCellGen/DepoChunker.h"
#include "WireCellIface/SimpleDepoSet.h"
#include "WireCellUtil/Units.h"

#include "WireCellUtil/NamedFactory.h"
WIRECELL_FACTORY(DepoChunker, WireCell::Gen::DepoChunker, WireCell::IDepoCollector, WireCell::IConfigurable)
using namespace std;
using namespace WireCell;

Gen::DepoChunker::DepoChunker()
  : m_count(0)
  , m_gate(0, 0)
  , m_starting_gate(0, 0)
{
}

Gen::DepoChunker::~DepoChunker() {}

WireCell::Configuration Gen::DepoChunker::default_configuration() const
{
    Configuration cfg;

    /// Set the time gate with a two element array of times.  Only
    /// depos within the gate are output.
    cfg["gate"] = Json::arrayValue;

    return cfg;
}

void Gen::DepoChunker::configure(const WireCell::Configuration& cfg)
{
    m_starting_gate = m_gate = std::pair<double, double>(cfg["gate"][0].asDouble(), cfg["gate"][1].asDouble());
}

void Gen::DepoChunker::emit(output_queue& out)
{
    out.push_back(std::make_shared<SimpleDepoSet>(m_count, m_depos));
    m_depos.clear();
    ++m_count;
}

bool Gen::DepoChunker::operator()(const input_pointer& depo, output_queue& deposetqueue)
{
    if (!depo) {  // EOS
        emit(deposetqueue);
        m_depos.push_back(depo);
        m_gate = m_starting_gate;
        return true;
    }

    const double now = depo->time();

    // inside current gate.
    if (m_gate.first <= now and now < m_gate.second) {
        m_depos.push_back(depo);
        return true;
    }

    if (now >= m_gate.second) {  // start new gate
        emit(deposetqueue);
        const double window = m_gate.second - m_gate.first;
        m_gate = std::pair<double, double>(m_gate.second, m_gate.second + window);
        m_depos.push_back(depo);
        return true;
    }

    std::cerr << "Gen::DepoChunker: out of time order depo received: now=" << now / units::s << "s\n";
    return false;
}
