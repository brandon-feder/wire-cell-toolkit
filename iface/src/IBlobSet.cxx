#include "WireCellIface/IBlobSet.h"

using namespace WireCell;

WireCell::RayGrid::blobs_t WireCell::IBlobSet::shapes() const
{
    RayGrid::blobs_t ret;
    for (const auto& ib : blobs()) {
        ret.push_back(ib->shape());
    }
    return ret;
}
