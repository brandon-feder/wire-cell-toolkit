#include "WireCellPgraph/Pgrapher.h"
#include "WireCellPgraph/Factory.h"
#include "WireCellIface/INode.h"
#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(Pgrapher, WireCell::Pgraph::Pgrapher, WireCell::IApplication, WireCell::IConfigurable)

using WireCell::get;
using namespace WireCell::Pgraph;

WireCell::Configuration Pgrapher::default_configuration() const
{
    Configuration cfg;

    cfg["edges"] = Json::arrayValue;
    return cfg;
}

static std::pair<WireCell::INode::pointer, int> get_node(WireCell::Configuration jone)
{
    using namespace WireCell;
    std::string node = jone["node"].asString();

    // We should NOT be the one creating this component.
    auto nptr = WireCell::Factory::find_maybe_tn<INode>(node);
    if (!nptr) {
        THROW(ValueError() << errmsg{"failed to get node"});
    }

    int port = get(jone, "port", 0);
    return std::make_pair(nptr, port);
}

void Pgrapher::configure(const WireCell::Configuration& cfg)

{
    Pgraph::Factory fac;
    l->debug("connecting: {} edges", cfg["edges"].size());
    for (auto jedge : cfg["edges"]) {
        auto tail = get_node(jedge["tail"]);
        auto head = get_node(jedge["head"]);

        SPDLOG_LOGGER_TRACE(l, "connecting: {}", jedge);

        bool ok = m_graph.connect(fac(tail.first), fac(head.first), tail.second, head.second);
        if (!ok) {
            l->critical("failed to connect edge: {}", jedge);
            THROW(ValueError() << errmsg{"failed to connect edge"});
        }
    }
    if (!m_graph.connected()) {
        l->critical("graph not fully connected");
        THROW(ValueError() << errmsg{"graph not fully connected"});
    }
}

void Pgrapher::execute()
{
    m_graph.execute();
    m_graph.print_timers();
}

Pgrapher::Pgrapher()
  : l(Log::logger("pgraph"))
{
}
Pgrapher::~Pgrapher() {}
