#include "WireCellIface/IWirePlane.h"

using namespace WireCell;

IWirePlane::~IWirePlane() {}

WirePlaneId IWirePlane::planeid() const
{
    static const WirePlaneId bogus(kUnknownLayer, -1, -1);
    const IWire::vector& w = wires();
    if (w.empty()) {
        return bogus;
    }
    return w.front()->planeid();
}
