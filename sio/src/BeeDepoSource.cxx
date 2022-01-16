#include "WireCellSio/BeeDepoSource.h"

#include "WireCellIface/IRecombinationModel.h"

#include "WireCellIface/SimpleDepo.h"
#include "WireCellUtil/NamedFactory.h"

#include "WireCellUtil/Point.h"
#include "WireCellUtil/Persist.h"

WIRECELL_FACTORY(BeeDepoSource, WireCell::Sio::BeeDepoSource, WireCell::IDepoSource, WireCell::IConfigurable)

#include <iostream>
#include <string>
#include <locale>  // for std::tolower

using namespace WireCell;

Sio::BeeDepoSource::BeeDepoSource()
  : m_policy("")
{
}

Sio::BeeDepoSource::~BeeDepoSource() {}

bool Sio::BeeDepoSource::operator()(IDepo::pointer& out)
{
    out = nullptr;

    if (m_depos.size() > 0) {
        out = m_depos.back();
        m_depos.pop_back();
        return true;
    }

    // refill
    while (!m_filenames.empty()) {
        std::string fname = m_filenames.back();
        m_filenames.pop_back();
        Json::Value jdat = Persist::load(fname);
        int ndepos = jdat["x"].size();
        if (!ndepos) {
            continue;
        }

        m_depos.resize(ndepos, nullptr);
        for (int idepo = 0; idepo < ndepos; ++idepo) {
            m_depos[idepo] = std::make_shared<SimpleDepo>(
                jdat["t"][idepo].asDouble(),
                Point(jdat["x"][idepo].asDouble(), jdat["y"][idepo].asDouble(), jdat["z"][idepo].asDouble()),
                jdat["q"][idepo].asDouble());
        }
        if (m_policy != "stream") {
            m_depos.push_back(nullptr);  // chunk by file
        }
        std::reverse(m_depos.begin(), m_depos.end());
        break;
    }

    if (m_depos.empty()) {
        return false;
    }

    out = m_depos.back();
    m_depos.pop_back();
    return true;
}

WireCell::Configuration Sio::BeeDepoSource::default_configuration() const
{
    Configuration cfg;
    cfg["filelist"] = Json::arrayValue;  // list of input files, empties are skipped
    cfg["policy"] = "";                  // set to "stream" to avoid sending EOS after each file's worth of depos.
    return cfg;
}

void Sio::BeeDepoSource::configure(const WireCell::Configuration& cfg)
{
    m_filenames = get<std::vector<std::string> >(cfg, "filelist");
    std::reverse(m_filenames.begin(), m_filenames.end());  // to use pop_back().
    m_policy = get<std::string>(cfg, "policy", "");
}
