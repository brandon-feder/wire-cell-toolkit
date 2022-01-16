#include "WireCellPgraph/Factory.h"
#include "WireCellPgraph/Graph.h"

#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDepoSink.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Type.h"

#include <iostream>

using namespace std;
using namespace WireCell;

int main()
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellPgraph");
    pm.add("WireCellGen");

    // User code normally does not see any construction/configuration
    // related stuff.  Don't replicate this in non-test code!
    {
        auto obj = Factory::lookup<IConfigurable>("TrackDepos");
        auto cfg = obj->default_configuration();
        cfg["tracks"][0]["time"] = 100.0 * units::ms;
        cfg["tracks"][0]["charge"] = 10000;
        cfg["tracks"][0]["ray"]["tail"]["x"] = 0.0;
        cfg["tracks"][0]["ray"]["tail"]["y"] = 0.0;
        cfg["tracks"][0]["ray"]["tail"]["z"] = 0.0;
        cfg["tracks"][0]["ray"]["head"]["x"] = 1 * units::cm;
        cfg["tracks"][0]["ray"]["head"]["y"] = 1 * units::cm;
        cfg["tracks"][0]["ray"]["head"]["z"] = 1 * units::cm;
        cerr << "TrackDepos cfg:\n" << cfg << endl;
        obj->configure(cfg);
    }
    // DumpDepos needs no config.

    // Pretend like we are some app with a hard-coded graph structure
    INode::pointer ds = Factory::lookup<INode>("TrackDepos");
    INode::pointer dd = Factory::lookup<INode>("DumpDepos");

    Pgraph::Factory pgwfac;

    auto dsnode = pgwfac(ds);
    cerr << "TrackDepos node is type: " << type(dsnode) << endl;

    auto ddnode = pgwfac(dd);
    cerr << "DumpDepos node is type: " << type(ddnode) << endl;

    Pgraph::Graph graph;
    graph.connect(dsnode, ddnode);
    graph.execute();

    return 0;
}
