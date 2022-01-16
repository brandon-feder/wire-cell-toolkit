#include "WireCellGen/DepoPlaneX.h"
#include "WireCellGen/TransportedDepo.h"

using namespace WireCell;

Gen::DepoPlaneX::DepoPlaneX(double planex, double speed)
  : m_planex(planex)
  , m_speed(speed)
  , m_queue(IDepoDriftCompare(speed))
{
}

IDepo::pointer Gen::DepoPlaneX::add(const IDepo::pointer& depo)
{
    drain(depo->time());
    IDepo::pointer newdepo(new TransportedDepo(depo, m_planex, m_speed));
    m_queue.insert(newdepo);
    return newdepo;
}

double Gen::DepoPlaneX::freezeout_time() const
{
    if (m_frozen.empty()) {
        return -1.0 * units::second;
    }
    return m_frozen.back()->time();
}

void Gen::DepoPlaneX::drain(double time)
{
    IDepo::vector doomed;
    for (auto depo : m_queue) {
        if (depo->time() < time) {
            m_frozen.push_back(depo);
            doomed.push_back(depo);
        }
    }
    for (auto depo : doomed) {
        m_queue.erase(depo);
    }
}

void Gen::DepoPlaneX::freezeout()
{
    IDepo::vector doomed;
    for (auto depo : m_queue) {
        m_frozen.push_back(depo);
        doomed.push_back(depo);
    }
    for (auto depo : doomed) {
        m_queue.erase(depo);
    }
}

IDepo::vector Gen::DepoPlaneX::pop(double time)
{
    drain(time);
    auto found =
        std::find_if_not(m_frozen.begin(), m_frozen.end(), [time](IDepo::pointer p) { return p->time() <= time; });
    IDepo::vector ret(m_frozen.begin(), found);
    m_frozen.erase(m_frozen.begin(), found);
    return ret;
}
