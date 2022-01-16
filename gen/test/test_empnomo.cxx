#include "WireCellUtil/Persist.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IChannelStatus.h"
#include "WireCellIface/IChannelSpectrum.h"

#include <cstdlib>
#include <string>

#include "anode_loader.h"  // ignore everything in this file

using namespace std;
using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string detector = "uboone";

    // In the real WCT this is done by wire-cell and driven by user
    // configuration.
    auto anode_tns = anode_loader(detector);

    cerr << "Using AnodePlane: \"" << anode_tns[0] << "\"\n";

    {
        {
            auto icfg = Factory::lookup<IConfigurable>("StaticChannelStatus");
            // In the real app this would be in a JSON or Jsonnet config
            // file in wire-cell-cfg.  Here, just to avoid an external
            // file we define a couple hard-coded deviant values somewhat
            // laboriously
            auto cfg = icfg->default_configuration();
            cfg["anode"] = anode_tns[0];
            // cfg["deviants"] = Json::arrayValue;
            // cfg["deviants"][0]["chid"] = 100;
            // cfg["deviants"][0]["gain"] = 4.7*units::mV/units::fC;
            // cfg["deviants"][0]["shaping"] = 1.0*units::us;
            // cfg["deviants"][1]["chid"] = 200;
            // cfg["deviants"][1]["gain"] = 4.7*units::mV/units::fC;
            // cfg["deviants"][1]["shaping"] = 1.0*units::us;
            icfg->configure(cfg);
        }
        {
            auto icfg = Factory::lookup<IConfigurable>("EmpiricalNoiseModel");
            auto cfg = icfg->default_configuration();
            cfg["anode"] = anode_tns[0];
            cfg["spectra_file"] = "microboone-noise-spectra-v2.json.bz2";
            icfg->configure(cfg);
        }
    }
    auto anode = Factory::find_tn<IAnodePlane>(anode_tns[0]);

    cerr << "Creating EmpiricalNoiseModel...\n";

    auto empnomo = Factory::find_tn<IChannelSpectrum>("EmpiricalNoiseModel");

    auto chids = anode->channels();
    cerr << "Got " << chids.size() << " channels\n";
    for (auto chid : chids) {
        const auto& amp = (*empnomo)(chid);
        double tot = 0;
        for (auto v : amp) {
            tot += v;
        }
        cerr << "ch:" << chid << " " << amp.size() << " tot=" << tot << endl;
    }

    return 0;
}
