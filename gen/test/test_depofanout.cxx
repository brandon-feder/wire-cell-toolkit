#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"
#include "WireCellIface/IDepoFanout.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/SimpleDepo.h"

using namespace WireCell;

int main()
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");

    const size_t multiplicity = 6;
    const std::string df_tn = "DepoFanout";

    {
        auto icfg = Factory::lookup<IConfigurable>(df_tn);
        auto cfg = icfg->default_configuration();
        cfg["multiplicity"] = (int) multiplicity;
        icfg->configure(cfg);
    }

    auto dfo = Factory::find_tn<IDepoFanout>(df_tn);

    Assert(dfo->output_types().size() == multiplicity);

    const int ident = 42;
    auto realdepo = std::make_shared<SimpleDepo>(0, Point(0, 0, 0), 1.0, nullptr, 0, 0, ident);

    for (auto depo : IDepo::vector({realdepo, nullptr})) {
        IDepoFanout::output_vector outv;
        bool ok = (*dfo)(depo, outv);
        Assert(ok);  // should always be so
        Assert(outv.size() == multiplicity);
        for (auto d : outv) {
            if (depo) {
                Assert(d->id() == ident);
            }
            else {
                Assert(d == nullptr);
            }
        }
    }
    return 0;
}
