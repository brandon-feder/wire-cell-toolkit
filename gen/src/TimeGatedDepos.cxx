#include "WireCellGen/TimeGatedDepos.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/String.h"

#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IAnodeFace.h"
#include "WireCellIface/IWirePlane.h"
#include "WireCellIface/SimpleDepo.h"

#include <boost/range.hpp>

#include <sstream>
#include <iostream>

WIRECELL_FACTORY(TimeGatedDepos, WireCell::Gen::TimeGatedDepos, WireCell::IDrifter, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

WireCell::Configuration Gen::TimeGatedDepos::default_configuration() const
{
    Configuration cfg;
    cfg["accept"] = m_accept;
    cfg["period"] = m_period;
    cfg["start"] = m_start;
    cfg["duration"] = m_duration;
    return cfg;
}

Gen::TimeGatedDepos::TimeGatedDepos()
  : m_accept(true)
  , m_period(0.0)
  , m_start(0.0)
  , m_duration(0.0)

{
}
Gen::TimeGatedDepos::~TimeGatedDepos() {}

bool Gen::TimeGatedDepos::operator()(const input_pointer& depo, output_queue& outq)
{
    if (!depo) {
        m_start += m_period;
        outq.push_back(nullptr);
        return true;
    }

    const double t = depo->time();
    const bool ingate = m_start <= t && t < m_start + m_duration;
    if (ingate == m_accept) {
        outq.push_back(depo);
    }
    return true;
}

void Gen::TimeGatedDepos::configure(const WireCell::Configuration& cfg)
{
    m_accept = cfg["accept"].asString() == "accept";
    m_period = get(cfg, "period", m_period);
    m_start = get(cfg, "start", m_start);
    m_duration = get(cfg, "duration", m_duration);
}
