#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Type.h"

using namespace std;
using namespace WireCell;

int main()
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");

    {
        auto icfg = Factory::lookup_tn<IConfigurable>("TrackDepos");
        auto cfg = icfg->default_configuration();
        icfg->configure(cfg);
    }

    auto ds = Factory::find_tn<IDepoSource>("TrackDepos");
    cerr << "TrackDepos is type: " << type(ds) << endl;

    Assert(ds);

    IDepo::pointer depo;
    bool ok = (*ds)(depo);

    Assert(ok);

    return 0;
}
