#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellGen/PlaneImpactResponse.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Logging.h"

#include <string>

using spdlog::debug;
using spdlog::error;
using namespace WireCell;

int main(int argc, char* argv[]) {
    Log::add_stdout(true, "debug");
    Log::set_level("debug");

    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    pm.add("WireCellSigProc");

    std::string response_file = "dune-garfield-1d565.json.bz2";
    if (argc > 1) {
        response_file = argv[1];
    };

    {
        auto icfg = Factory::lookup<IConfigurable>("FieldResponse");
        auto cfg = icfg->default_configuration();
        cfg["filename"] = response_file;
        icfg->configure(cfg);
    }

    Gen::PlaneImpactResponse pir;
    auto cfg = pir.default_configuration();
    cfg["plane"] = 0;
    cfg["nticks"] = 9595;
    cfg["tick"] = 0.5 * units::us;
    pir.configure(cfg);

    const auto& irs = pir.irs();
    const auto& bywire_map = pir.bywire_map();
    for (int iwire=0; iwire<pir.nwires(); ++iwire) {
        const auto& ri = bywire_map[iwire];
        Assert(ri.size() == 11); // number of imps spanning a wire region
        for (size_t ind=0; ind<ri.size(); ++ind) {
            size_t ri_ind = ri[ind];
            const auto ir = irs[ind];
            debug("iwire:{} ind:{} ri_ind:{} imp:{}", iwire, ind, ri_ind, ir->impact());
        }
        
    }
    return 0;
}
