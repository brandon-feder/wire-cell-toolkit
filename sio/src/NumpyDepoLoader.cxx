#include "NumpyDepoTools.h"
#include "WireCellSio/NumpyDepoLoader.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Array.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/NumpyHelper.h"
#include "WireCellAux/Logger.h"
#include "WireCellIface/SimpleDepo.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <tuple>

WIRECELL_FACTORY(NumpyDepoLoader, WireCell::Sio::NumpyDepoLoader,
                 WireCell::IDepoSource, WireCell::IConfigurable)

using namespace WireCell;

Sio::NumpyDepoLoader::NumpyDepoLoader()
    : Aux::Logger("NumpyDepoLoader", "io")
    , m_load_count(0)
    , m_eos(1)
{
}

Sio::NumpyDepoLoader::~NumpyDepoLoader() {}

WireCell::Configuration Sio::NumpyDepoLoader::default_configuration() const
{
    Configuration cfg;

    // The input file name to read. 
    cfg["filename"] = "wct-frame.npz";

    return cfg;
}

void Sio::NumpyDepoLoader::configure(const WireCell::Configuration& config)
{
    m_cfg = config;
}



using DataArray = Eigen::Array<float, Eigen::Dynamic, 7>;
using InfoArray = Eigen::Array<int, Eigen::Dynamic, 4>;

bool Sio::NumpyDepoLoader::next()
{
    const int load_count = m_load_count++;

    IDepo::vector depos;
    bool ok = Sio::NumpyDepoTools::load(m_cfg["filename"].asString(),
                                        load_count, depos);
    if (!ok) {
        return false;
    }

    m_depos.insert(m_depos.end(), depos.begin(), depos.end());
    log->debug("load {} complete with {} new, {} total",
               load_count, depos.size(), m_depos.size());

    return true;
}

bool Sio::NumpyDepoLoader::operator()(WireCell::IDepo::pointer& outdepo)
{
    outdepo = nullptr;
    if (m_depos.empty()) {
        if (m_eos == 0) {
            log->debug("send EOS");
            m_eos = 1;          // we just finished a stream
            return true;        // so send null outdepo -> EOS
        }
        // We are outside a stream, try to initiate a new one
        bool ok = next();
        if (!ok or m_depos.empty()) { // failed to initiate
            log->warn("depo stream ended at call={}", m_load_count);
            m_depos.clear();
            return false;       // the stream is dry
        }
        m_eos = 0;              // we are in a working stream
    }

    outdepo = m_depos.front();
    m_depos.pop_front();
    if (!outdepo) {
        log->debug("got no depo");
    }
    // else {  // super verbose!
    //     log->debug("depo loader: t={} q={} x={}",
    //                outdepo->time(), outdepo->charge(), outdepo->pos().x());
    // }
    return true;
}
