#include "WireCellApps/NodeDumper.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Type.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/Persist.h"
#include "WireCellIface/INode.h"
#include "WireCellUtil/Logging.h"

WIRECELL_FACTORY(NodeDumper, WireCellApps::NodeDumper, WireCell::IApplication, WireCell::IConfigurable)

using spdlog::info;
using spdlog::warn;

using namespace std;
using namespace WireCell;
using namespace WireCellApps;

NodeDumper::NodeDumper()
  : m_cfg(default_configuration())
{
}

NodeDumper::~NodeDumper() {}

void NodeDumper::configure(const Configuration& config) { m_cfg = config; }

WireCell::Configuration NodeDumper::default_configuration() const
{
    Configuration cfg;
    cfg["filename"] = "/dev/stdout";
    cfg["nodes"] = Json::arrayValue;
    return cfg;
}

void NodeDumper::execute()
{
    Configuration all;

    int nnodes = m_cfg["nodes"].size();
    std::vector<std::string> types;
    if (0 == nnodes) {
        types = Factory::known_classes<INode>();
        nnodes = types.size();
        info("Dumping all known node classes ({})", nnodes);
    }
    else {
        info("Dumping: ({})", nnodes);
        for (auto type_cfg : m_cfg["nodes"]) {
            std::string type = convert<string>(type_cfg);
            types.push_back(type);
            info("\t{}", type);
        }
    }

    for (auto type : types) {
        INode::pointer node;
        try {
            node = Factory::lookup<INode>(type);
        }
        catch (FactoryException& fe) {
            warn("NodeDumper: failed lookup node: \"{}\"", type);
            continue;
        }

        Configuration one;
        one["type"] = type;
        for (auto intype : node->input_types()) {
            one["input_types"].append(demangle(intype));
        }
        for (auto intype : node->output_types()) {
            one["output_types"].append(demangle(intype));
        }
        one["concurrency"] = node->concurrency();
        one["category"] = node->category();

        all.append(one);
    }

    Persist::dump(get<string>(m_cfg, "filename"), all);
}
