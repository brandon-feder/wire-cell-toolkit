#include "WireCellGen/DepoFanout.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

WIRECELL_FACTORY(DepoFanout, WireCell::Gen::DepoFanout,
                 WireCell::INamed,
                 WireCell::IDepoFanout, WireCell::IConfigurable)

using namespace WireCell;
using namespace std;

Gen::DepoFanout::DepoFanout(size_t multiplicity)
  : Aux::Logger("DepoFanout", "gen")
  , m_multiplicity(multiplicity)
{
}

Gen::DepoFanout::~DepoFanout() {}

WireCell::Configuration Gen::DepoFanout::default_configuration() const
{
    Configuration cfg;
    cfg["multiplicity"] = (int) m_multiplicity;
    return cfg;
}

void Gen::DepoFanout::configure(const WireCell::Configuration& cfg)
{
    int m = get<int>(cfg, "multiplicity", (int) m_multiplicity);
    if (m <= 0) {
        THROW(ValueError() << errmsg{"DepoFanout multiplicity must be positive"});
    }
    m_multiplicity = m;
}

std::vector<std::string> Gen::DepoFanout::output_types()
{
    const std::string tname = std::string(typeid(output_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool Gen::DepoFanout::operator()(const input_pointer& in, output_vector& outv)
{
    if (!in) {
        log->debug("EOS");
        // fall through
    }

    outv.resize(m_multiplicity);
    for (size_t ind = 0; ind < m_multiplicity; ++ind) {
        outv[ind] = in;
    }

    return true;
}
