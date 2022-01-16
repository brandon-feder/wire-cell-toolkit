#include "WireCellGen/DepoBagger.h"
#include "WireCellIface/SimpleDepoSet.h"
#include "WireCellUtil/Logging.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(DepoBagger, WireCell::Gen::DepoBagger,
                 WireCell::INamed,
                 WireCell::IDepoCollector, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::DepoBagger::DepoBagger()
  : Aux::Logger("DepoBagger", "gen")
{
}

Gen::DepoBagger::~DepoBagger() {}

WireCell::Configuration Gen::DepoBagger::default_configuration() const
{
    Configuration cfg;

    /// Set the time gate with a two element array of times.  Only
    /// depos within the gate are output.
    cfg["gate"] = Json::arrayValue;

    return cfg;
}

void Gen::DepoBagger::configure(const WireCell::Configuration& cfg)
{
    if (! cfg["gate"].isNull()) {
        m_gate = std::pair<double, double>(
            cfg["gate"][0].asDouble(), cfg["gate"][1].asDouble());
    }
}

bool Gen::DepoBagger::operator()(const input_pointer& depo, output_queue& deposetqueue)
{
    if (!depo) {  // EOS
        log->debug("send bag #{} with {} depos followed by EOS",
                   m_count, m_depos.size());
        // even if empty, must send out something to retain sync.
        auto out = std::make_shared<SimpleDepoSet>(m_count, m_depos);
        deposetqueue.push_back(out);
        m_depos.clear();
        deposetqueue.push_back(nullptr);  // pass on EOS
        ++m_count;
        return true;
    }

    const double t = depo->time();
    if (m_gate.first == 0.0 and m_gate.second == 0.0) {
        m_depos.push_back(depo);
    }
    else if (m_gate.first <= t and t < m_gate.second) {
        m_depos.push_back(depo);
    }
    return true;
}
