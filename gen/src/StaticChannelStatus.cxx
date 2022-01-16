#include "WireCellGen/StaticChannelStatus.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(StaticChannelStatus, WireCell::Gen::StaticChannelStatus, WireCell::IChannelStatus,
                 WireCell::IConfigurable)

using namespace WireCell;

Gen::StaticChannelStatus::StaticChannelStatus(double nominal_gain, double nominal_shaping,
                                              channel_status_map_t deviants)
  : m_nominal_gain(nominal_gain)
  , m_nominal_shaping(nominal_shaping)
  , m_deviants(deviants)
{
}

Gen::StaticChannelStatus::~StaticChannelStatus() {}

WireCell::Configuration Gen::StaticChannelStatus::default_configuration() const
{
    Configuration cfg;  // load hard-coded defaults
    cfg["nominal_gain"] = m_nominal_gain;
    cfg["nominal_shaping"] = m_nominal_shaping;
    cfg["deviants"] = Json::arrayValue;
    for (auto it : m_deviants) {
        Json::Value jdev(Json::objectValue);
        jdev["chid"] = it.first;
        jdev["gain"] = it.second.gain;
        jdev["shaping"] = it.second.shaping;
        cfg["deviants"].append(jdev);
    }
    return cfg;
}

void Gen::StaticChannelStatus::configure(const WireCell::Configuration& cfg)
{
    // let user override any defaults
    m_nominal_gain = get(cfg, "nominal_gain", m_nominal_gain);
    m_nominal_shaping = get(cfg, "nominal_shaping", m_nominal_shaping);

    auto jdev = cfg["deviants"];
    if (jdev.isNull()) {
        return;
    }
    for (auto jone : jdev) {
        const int chid = jone["chid"].asInt();  // must supply
        const double gain = get(jone, "gain", m_nominal_gain);
        const double shaping = get(jone, "shaping", m_nominal_shaping);
        m_deviants[chid] = ChannelStatus(gain, shaping);
    }
}

double Gen::StaticChannelStatus::preamp_gain(int chid) const
{
    auto it = m_deviants.find(chid);
    if (it == m_deviants.end()) {
        return m_nominal_gain;
    }
    return it->second.gain;
}
double Gen::StaticChannelStatus::preamp_shaping(int chid) const
{
    auto it = m_deviants.find(chid);
    if (it == m_deviants.end()) {
        return m_nominal_shaping;
    }
    return it->second.shaping;
}
