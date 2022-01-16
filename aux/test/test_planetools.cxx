#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellAux/PlaneTools.h"

#include <iostream>
#include <vector>
#include <unordered_set>

using namespace WireCell;

int main()
{
    auto params = Persist::load("pgrapher/experiment/pdsp/params.jsonnet");

    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    {
        auto icfg = Factory::lookup_tn<IConfigurable>("WireSchemaFile");
        auto cfg = icfg->default_configuration();
        cfg["filename"] = "protodune-wires-larsoft-v4.json.bz2";
        icfg->configure(cfg);
    }
    {
        auto icfg = Factory::lookup_tn<IConfigurable>("AnodePlane");
        auto cfg = icfg->default_configuration();
        cfg["ident"] = 0;
        cfg["wire_schema"] ="WireSchemaFile";
        cfg["faces"] = params["det"]["volumes"][0]["faces"];
        icfg->configure(cfg);
    }

    auto anode = Factory::find_tn<IAnodePlane>("AnodePlane");
    assert(anode);

    const std::vector<size_t> sizes{800,800,960};

    for (int wpi=0; wpi<3; ++wpi) {
        auto ichans = Aux::plane_channels(anode, wpi);
        std::cerr << wpi << " " << ichans.size() << "\n";
        assert (ichans.size() == sizes[wpi]);
        std::unordered_set<int> chanset;
        for (auto ichan : ichans) {
            std::cerr << ichan->ident() << " ";
            chanset.insert(ichan->ident());
        }
        assert (chanset.size() == sizes[wpi]); // no dupes!
        std::cerr << "\n";
    }

    return 0;    
}
