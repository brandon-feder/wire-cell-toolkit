#include "WireCellIface/IDepo.h"
using namespace WireCell;

IDepo::vector WireCell::depo_chain(IDepo::pointer last)
{
    IDepo::vector ret;
    while (true) {
        ret.push_back(last);
        last = last->prior();
        if (!last) {
            break;
        }
    }
    return ret;
}

bool WireCell::ascending_time(const WireCell::IDepo::pointer& lhs, const WireCell::IDepo::pointer& rhs)
{
    if (lhs->time() == rhs->time()) {
        if (lhs->pos().x() == lhs->pos().x()) {
            return lhs.get() < rhs.get();  // break tie by pointer
        }
        return lhs->pos().x() < lhs->pos().x();
    }
    return lhs->time() < rhs->time();
}

/// Compare two IDepo::pointers for by time, descending.   x is used to break tie
bool WireCell::descending_time(const WireCell::IDepo::pointer& lhs, const WireCell::IDepo::pointer& rhs)
{
    if (lhs->time() == rhs->time()) {
        if (lhs->pos().x() == lhs->pos().x()) {
            return lhs.get() > rhs.get();  // break tie by pointer
        }
        return lhs->pos().x() > lhs->pos().x();
    }
    return lhs->time() > rhs->time();
}
