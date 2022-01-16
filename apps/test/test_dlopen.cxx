#include "WireCellIface/IWireParameters.h"

#include "WireCellUtil/Singleton.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"

#include <dlfcn.h>

#include <iostream>
using namespace std;
using namespace WireCell;

int main()
{
    PluginManager& pm = PluginManager::instance();
    Plugin* gen_plug = pm.add("WireCellGen");
    Assert(gen_plug);

    typedef NamedFactoryRegistry<IWireParameters> IWPFactoryRegistry;
    IWPFactoryRegistry* iwp1 = &Singleton<IWPFactoryRegistry>::Instance();
    IWPFactoryRegistry* iwp2 = &Singleton<IWPFactoryRegistry>::Instance();
    Assert(iwp1);
    Assert(iwp2);
    Assert(iwp1 == iwp2);

    cerr << "Looking up WireParams factory in NFR: " << iwp1 << endl;
    auto inst1 = iwp1->lookup_factory("WireParams");
    auto inst2 = iwp1->lookup_factory("WireParams");

    Assert(inst1);
    Assert(inst2);
    Assert(inst1 == inst2);

    cerr << "Looking up WireParams factory \n";
    auto factory = Factory::lookup_factory<IWireParameters>("WireParams");
    Assert(factory);

    cerr << "Looking up WireParams:\n";
    auto wp1 = Factory::lookup<IWireParameters>("WireParams");
    Assert(wp1);
    cerr << "Looking up WireParams, again:\n";
    auto wp2 = Factory::lookup<IWireParameters>("WireParams");
    Assert(wp2);
    cerr << "Looking up undefined WireParams\n";
    auto wp3 = Factory::lookup<IWireParameters>("WireParams", "MyWireParameters");
    Assert(wp3);

    Assert(wp2 == wp1);
    Assert(wp3 != wp1);

    return 0;
}
