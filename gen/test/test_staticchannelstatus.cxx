#include "WireCellGen/StaticChannelStatus.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/PluginManager.h"

#include <iostream>

using namespace std;
using namespace WireCell;

// These are supposed to be the hard-coded defaults in the class.
const double nominal_gain = 14.0 * units::mV / units::fC;
const double nominal_shaping = 2.0 * units::us;

const double other_gain = 4.7 * units::mV / units::fC;
const double other_shaping = 1.0 * units::us;

Gen::StaticChannelStatus::channel_status_map_t make_deviants()
{
    Gen::StaticChannelStatus::channel_status_map_t deviants;
    deviants[11] = Gen::StaticChannelStatus::ChannelStatus(other_gain, other_shaping);
    deviants[12] = Gen::StaticChannelStatus::ChannelStatus(other_gain, nominal_shaping);
    deviants[13] = Gen::StaticChannelStatus::ChannelStatus(nominal_gain, other_shaping);
    deviants[14] = Gen::StaticChannelStatus::ChannelStatus(nominal_gain, nominal_shaping);
    return deviants;
}

void test_hardcode()
{
    auto deviants = make_deviants();

    Gen::StaticChannelStatus scs(nominal_gain, nominal_shaping, deviants);

    for (int chid = 0; chid < 10; ++chid) {
        Assert(scs.preamp_gain(chid) == nominal_gain);
        Assert(scs.preamp_shaping(chid) == nominal_shaping);
    }

    for (int chid = 11; chid <= 14; ++chid) {
        Assert(scs.preamp_gain(chid) == deviants[chid].gain);
        Assert(scs.preamp_shaping(chid) == deviants[chid].shaping);
    }
}

void test_config()
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");

    auto cs = Factory::lookup<IChannelStatus>("StaticChannelStatus");
    auto cscfg = Factory::lookup<IConfigurable>("StaticChannelStatus");

    auto deviants = make_deviants();

    Configuration cfg = cscfg->default_configuration();
    cfg["deviants"] = Json::arrayValue;
    for (auto it : deviants) {
        Json::Value jone(Json::objectValue);
        jone["chid"] = it.first;
        jone["gain"] = it.second.gain;
        jone["shaping"] = it.second.shaping;
        cfg["deviants"].append(jone);
    }
    cscfg->configure(cfg);

    for (int chid = 0; chid < 10; ++chid) {
        Assert(cs->preamp_gain(chid) == nominal_gain);
        Assert(cs->preamp_shaping(chid) == nominal_shaping);
    }

    for (int chid = 11; chid <= 14; ++chid) {
        // cerr << chid << ": "
        //      << " want gain: " << deviants[chid].gain
        //      << " got gain: " << cs->preamp_gain(chid)
        //      << endl;
        Assert(cs->preamp_gain(chid) == deviants[chid].gain);
        Assert(cs->preamp_shaping(chid) == deviants[chid].shaping);
    }
}

int main()
{
    test_hardcode();
    test_config();
    return 0;
}
