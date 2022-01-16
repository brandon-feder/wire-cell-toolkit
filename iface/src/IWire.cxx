#include "WireCellIface/IWire.h"

WireCell::IWire::~IWire() {}

WireCell::Point WireCell::IWire::center() const
{
    Ray seg = ray();
    return 0.5 * (seg.first + seg.second);
}

bool WireCell::ascending_index(IWire::pointer lhs, IWire::pointer rhs)
{
    if (lhs->planeid() == rhs->planeid()) {
        return lhs->index() < rhs->index();
    }
    return lhs->planeid() < lhs->planeid();
}

// bool WireCell::WirePlaneIndexCompare ::operator() (Wire lhs, Wire rhs) const
// {
//     return lhs < rhs;
// }

// bool operator==(WireCell::Wire lhs, WireCell::Wire rhs)
// {
//     return lhs->ident() == rhs->ident();
// }

// bool operator<(WireCell::Wire lhs, WireCell::Wire rhs)
// {
//     if (lhs->plane() == rhs->plane()) {
// 	return lhs->index() < rhs->index();
//     }
//     return lhs->plane() < rhs->plane();
// }

// std::ostream & operator<<(std::ostream &os, WireCell::Wire wire)
// {
//     os << "<WIRE " << wire->ident() << ">";
// }
