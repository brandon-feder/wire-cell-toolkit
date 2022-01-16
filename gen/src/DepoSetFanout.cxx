#include "WireCellGen/DepoSetFanout.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

WIRECELL_FACTORY(DepoSetFanout, WireCell::Gen::DepoSetFanout,
                 WireCell::INamed,
                 WireCell::IDepoSetFanout, WireCell::IConfigurable)

using namespace WireCell;
using namespace std;

Gen::DepoSetFanout::DepoSetFanout(size_t multiplicity)
    : Aux::Logger("DepoSetFanout", "glue")
    , m_multiplicity(multiplicity)
{
}

Gen::DepoSetFanout::~DepoSetFanout() {}

WireCell::Configuration Gen::DepoSetFanout::default_configuration() const
{
    Configuration cfg;
    cfg["multiplicity"] = (int) m_multiplicity;
    return cfg;
}

void Gen::DepoSetFanout::configure(const WireCell::Configuration& cfg)
{
    int m = get<int>(cfg, "multiplicity", (int) m_multiplicity);
    if (m <= 0) {
        log->critical("multiplicity must be positive, got {}", m);
        THROW(ValueError() << errmsg{"DepoSetFanout multiplicity must be positive"});
    }
    m_multiplicity = m;
}

std::vector<std::string> Gen::DepoSetFanout::output_types()
{
    const std::string tname = std::string(typeid(output_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool Gen::DepoSetFanout::operator()(const input_pointer& in, output_vector& outv)
{
    // Note: if "in" indicates EOS, just pass it on
    if (in) {
        log->debug("call={} fanout depo set {} with {}",
                   m_count, in->ident(), in->depos()->size());
    }
    else {
        log->debug("EOS at call={}", m_count);
    }

    outv.resize(m_multiplicity, nullptr);
    for (size_t ind = 0; ind < m_multiplicity; ++ind) {
        outv[ind] = in;
    }

    ++m_count;
    return true;
}
