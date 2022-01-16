#include "NumpyDepoTools.h"

#include "WireCellSio/NumpyDepoSetLoader.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleDepoSet.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <tuple>

WIRECELL_FACTORY(NumpyDepoSetLoader, WireCell::Sio::NumpyDepoSetLoader,
                 WireCell::INamed,
                 WireCell::IDepoSetSource, WireCell::IConfigurable)

using namespace WireCell;

Sio::NumpyDepoSetLoader::NumpyDepoSetLoader()
    : Aux::Logger("NumpyDepoSetLoader", "io")
{
}

Sio::NumpyDepoSetLoader::~NumpyDepoSetLoader() {}

WireCell::Configuration Sio::NumpyDepoSetLoader::default_configuration() const
{
    Configuration cfg;

    // The input file name to read. 
    cfg["filename"] = "wct-depos.npz";

    return cfg;
}

void Sio::NumpyDepoSetLoader::configure(const WireCell::Configuration& config)
{
    m_cfg = config;
}

bool Sio::NumpyDepoSetLoader::operator()(WireCell::IDepoSet::pointer& outdepos)
{
    outdepos = nullptr;    

    if (m_sent_eos) {
        log->debug("empty at call={}", m_load_count);
        return false;
    }

    IDepo::vector depos;
    bool ok = Sio::NumpyDepoTools::load(m_cfg["filename"].asString(),
                                        m_load_count, depos);
    if (!ok) {
        log->debug("send EOS at call={}", m_load_count);
        m_sent_eos = true;
        ++m_load_count;
        return true;
    }

    outdepos = std::make_shared<SimpleDepoSet>(m_load_count, depos);
    log->debug("call={} sent ndepos={}", m_load_count, depos.size());
    ++m_load_count;
    return true;
}
