#include "WireCellAux/TensorSetUnpacker.h"
#include "WireCellAux/TensUtil.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"

WIRECELL_FACTORY(TensorSetUnpacker, WireCell::Aux::TensorSetUnpacker, WireCell::ITensorSetUnpacker,
                 WireCell::IConfigurable)

using namespace WireCell;

Aux::TensorSetUnpacker::TensorSetUnpacker(size_t multiplicity)
  : m_multiplicity(multiplicity)
  , log(Log::logger("sig"))
{
}
Aux::TensorSetUnpacker::~TensorSetUnpacker() {}

WireCell::Configuration Aux::TensorSetUnpacker::default_configuration() const
{
    Configuration cfg;
    // How many output ports
    cfg["multiplicity"] = (int) m_multiplicity;
    //
    cfg["tags"] = Json::arrayValue;
    cfg["types"] = Json::arrayValue;
    return cfg;
}
void Aux::TensorSetUnpacker::configure(const WireCell::Configuration& cfg)
{
    m_cfg = cfg;
    auto tags = cfg["tags"];

    int m = tags.size();
    if (m <= 0) {
        THROW(ValueError() << errmsg{"TensorSetUnpacker multiplicity must be positive"});
    }
    m_multiplicity = m;
}

std::vector<std::string> Aux::TensorSetUnpacker::output_types()
{
    const std::string tname = std::string(typeid(output_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool Aux::TensorSetUnpacker::operator()(const input_pointer& in, output_vector& outv)
{
    outv.resize(m_multiplicity);

    if (!in) {  //  pass on EOS
        for (size_t ind = 0; ind < m_multiplicity; ++ind) {
            outv[ind] = nullptr;
        }
        log->debug("TensorSetUnpacker: see EOS");
        return true;
    }

    std::vector<std::string> tags;
    for (auto j : m_cfg["tags"]) {
        tags.push_back(j.asString());
    }
    std::vector<std::string> types;
    for (auto j : m_cfg["types"]) {
        types.push_back(j.asString());
    }

    for (size_t ind = 0; ind < m_multiplicity; ++ind) {
        auto iten = Aux::get_tens(in, tags[ind], types[ind]);
        outv[ind] = iten;
    }

    return true;
}
