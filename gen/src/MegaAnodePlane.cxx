#include "WireCellGen/MegaAnodePlane.h"
#include "WireCellUtil/NamedFactory.h"
#include <string>

WIRECELL_FACTORY(MegaAnodePlane, WireCell::Gen::MegaAnodePlane, WireCell::IAnodePlane, WireCell::IConfigurable)

using namespace WireCell;
using namespace std;

WireCell::Configuration Gen::MegaAnodePlane::default_configuration() const
{
    Configuration cfg;
    /// These must be provided
    cfg["anodes_tn"] = Json::arrayValue;

    return cfg;
}

void Gen::MegaAnodePlane::configure(const WireCell::Configuration& cfg)
{
    m_anodes.clear();
    auto anodes_tn = cfg["anodes_tn"];
    for (auto anode_tn : anodes_tn) {
        auto anode = Factory::find_tn<IAnodePlane>(anode_tn.asString());
        m_anodes.push_back(anode);
        // std::cout << "MegaAnodePlane: adding anode " << anode_tn << std::endl;
        // for(int channel: anode->channels()){
        // 	cout << channel << " ";
        // }
    }
}

WirePlaneId Gen::MegaAnodePlane::resolve(int channel) const
{
    // cout << "MegaAnodePlane: resolve channel " << channel << endl;
    const WirePlaneId bogus(0xFFFFFFFF);  // -1 is unknown

    for (auto& anode : m_anodes) {
        WirePlaneId planeId = anode->resolve(channel);
        if (planeId.index() > -1) {
            // std::cout << "MegaAnodePlane: plane index " << planeId.index() << "for channel " << channel << std::endl;
            return planeId;
        }
    }
    // cout << "MegaAnodePlane: unknown plane for channel " << channel << endl;
    return bogus;
}

std::vector<int> Gen::MegaAnodePlane::channels() const
{
    std::vector<int> ret;
    for (auto& anode : m_anodes) {
        auto chans = anode->channels();
        ret.insert(ret.end(), chans.begin(), chans.end());
    }
    return ret;
}

IChannel::pointer Gen::MegaAnodePlane::channel(int chident) const
{
    for (auto& anode : m_anodes) {
        auto ch = anode->channel(chident);
        if (ch == nullptr) {
            continue;
        }
        return ch;
    }
    return nullptr;
}

IWire::vector Gen::MegaAnodePlane::wires(int channel) const
{
    for (auto& anode : m_anodes) {
        auto ws = anode->wires(channel);
        if (ws.empty()) {
            continue;
        }
        return ws;
    }
    return IWire::vector();
}
