#include "WireCellSio/ClusterFileSink.h"

#include "WireCellAux/ClusterHelpers.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/NamedFactory.h"

#include "WireCellAux/FrameTools.h"

// This is found at *compile* time in util/inc/.
#include "custard/custard_boost.hpp"

WIRECELL_FACTORY(ClusterFileSink, WireCell::Sio::ClusterFileSink,
                 WireCell::INamed,
                 WireCell::IClusterSink, WireCell::ITerminal,
                 WireCell::IConfigurable)

using namespace WireCell;

Sio::ClusterFileSink::ClusterFileSink()
    : Aux::Logger("ClusterFileSink", "io")
    , m_drift_speed(1.6 * units::mm / units::us)
{
}

Sio::ClusterFileSink::~ClusterFileSink()
{
}

void Sio::ClusterFileSink::finalize()
{
    log->debug("closing {} after {} calls", m_outname, m_count);
    m_out.pop();
}

WireCell::Configuration Sio::ClusterFileSink::default_configuration() const
{
    Configuration cfg;
    // output json file.  A "%d" type format code may be included to be resolved by a cluster identifier.
    cfg["outname"] = m_outname;

    // Whether to also output any referenced frames.
    // cfg["output_frame"] = m_output_frame;

    // for conversion between time and "x" coordinate
    cfg["drift_speed"] = m_drift_speed;
    return cfg;
}

void Sio::ClusterFileSink::configure(const WireCell::Configuration& cfg)
{
    m_outname = get(cfg, "outname", m_outname);
    // m_output_frame = get(cfg, "output_frame", m_output_frame);
    m_drift_speed = get(cfg, "drift_speed", m_drift_speed);

    m_out.clear();
    custard::output_filters(m_out, m_outname);
    if (m_out.empty()) {
        THROW(ValueError() << errmsg{"ClusterFileSink: unsupported outname: " + m_outname});
    }
}

bool Sio::ClusterFileSink::operator()(const ICluster::pointer& cluster)
{
    if (!cluster) {             // EOS
        log->debug("see EOS at call={}", m_count);
        ++m_count;
        return true;
    }
    
    auto frame = Aux::find_frame(cluster);
    auto cname = Aux::name(frame);
    cname += "_";
    cname += Aux::name(cluster);
    cname += ".json";

    auto top = Aux::jsonify(cluster, m_drift_speed);
    std::stringstream topss;
    topss << top;
    auto tops = topss.str();


    log->debug("call={} output {} with {} bytes to {}",
               m_count, cname, tops.size(), m_outname );

    m_out << "name " << cname << "\n"
          << "body " << tops.size() << "\n" << tops.data();
    m_out.flush();

    ++m_count;
    return true;
}
