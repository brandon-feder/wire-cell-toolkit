#include "WireCellIface/IChannel.h"

using namespace WireCell;

IChannel::~IChannel() {}

WirePlaneId IChannel::planeid() const
{
    static const WirePlaneId bogus(kUnknownLayer, -1, -1);
    const IWire::vector& w = wires();
    if (w.empty()) {
        return bogus;
    }
    return w.front()->planeid();
}
