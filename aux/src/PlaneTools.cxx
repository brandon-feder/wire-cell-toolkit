#include "WireCellAux/PlaneTools.h"

using namespace WireCell;

IChannel::vector Aux::plane_channels(IAnodePlane::pointer anode,
                                     int wire_plane_index)
{
    IChannel::vector ret;    
    for (auto face : anode->faces()) {
        if (!face) {   // A null face means one sided AnodePlane.
            continue;  // Can be "back" or "front" face.
        }
        for (auto plane : face->planes()) {
            if (wire_plane_index != plane->planeid().index()) {
                continue;
            }
            // These IChannel vectors are ordered in same order as wire-in-plane.
            const auto& ichans = plane->channels();
            // Append
            ret.reserve(ret.size() + ichans.size());
            ret.insert(ret.end(), ichans.begin(), ichans.end());
        }
    }
    return ret;
}
