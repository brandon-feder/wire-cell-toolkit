#include "WireCellGen/DepoSetDrifter.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleDepoSet.h"

WIRECELL_FACTORY(DepoSetDrifter, WireCell::Gen::DepoSetDrifter,
                 WireCell::INamed,
                 WireCell::IDepoSetFilter, WireCell::IConfigurable)


using namespace WireCell;
using namespace WireCell::Gen;

DepoSetDrifter::DepoSetDrifter()
    : Aux::Logger("DepoSetDrifter", "gen")
{
}
DepoSetDrifter::~DepoSetDrifter()
{
}

WireCell::Configuration DepoSetDrifter::default_configuration() const
{
    Configuration cfg;
    // The typename of the drifter to do the real work.
    cfg["drifter"] = "Drifter";
    return cfg;
}

void DepoSetDrifter::configure(const WireCell::Configuration& cfg)
{
    auto name = get<std::string>(cfg, "drifter", "Drifter");
    m_drifter = Factory::find_tn<IDrifter>(name);
}

bool DepoSetDrifter::operator()(const input_pointer& in, output_pointer& out)
{
    out = nullptr;
    if (!in) {                  // EOS
        log->debug("EOS at call={}", m_count);
        return true;
    }

    // make a copy so we can append an EOS to flush the per depo
    // drifter.
    IDepo::vector in_depos(in->depos()->begin(), in->depos()->end());
    in_depos.push_back(nullptr); // input EOS

    IDepo::vector all_depos;
    for (auto idepo : in_depos) {
        IDrifter::output_queue more;        
        (*m_drifter)(idepo, more);
        all_depos.insert(all_depos.end(), more.begin(), more.end());
    }
    // The EOS comes through
    all_depos.pop_back();
        
    log->debug("call={} drifted ndepos={}", m_count, all_depos.size());
    out = std::make_shared<SimpleDepoSet>(m_count, all_depos);
    ++m_count;
    return true;
}

