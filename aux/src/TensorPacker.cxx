#include "WireCellAux/TensorPacker.h"
#include "WireCellAux/TensUtil.h"
#include "WireCellAux/SimpleTensorSet.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"

WIRECELL_FACTORY(TensorPacker, WireCell::Aux::TensorPacker, WireCell::ITensorPacker, WireCell::IConfigurable)

using namespace WireCell;

Aux::TensorPacker::TensorPacker(size_t multiplicity)
  : m_multiplicity(multiplicity)
  , log(Log::logger("sig"))
{
}
Aux::TensorPacker::~TensorPacker() {}

WireCell::Configuration Aux::TensorPacker::default_configuration() const
{
    Configuration cfg;
    // How many input ports
    cfg["multiplicity"] = (int) m_multiplicity;
    return cfg;
}
void Aux::TensorPacker::configure(const WireCell::Configuration& cfg)
{
    m_cfg = cfg;
    auto m = get<int>(cfg, "multiplicity", m_multiplicity);
    if (m <= 0) {
        THROW(ValueError() << errmsg{"TensorPacker multiplicity must be positive"});
    }
    m_multiplicity = m;
}

std::vector<std::string> Aux::TensorPacker::input_types()
{
    const std::string tname = std::string(typeid(input_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool Aux::TensorPacker::operator()(const input_vector& invec, output_pointer& out)
{
    out = nullptr;
    size_t neos = 0;
    for (const auto& fr : invec) {
        if (!fr) {
            ++neos;
        }
    }
    if (neos == invec.size()) {
        return true;
    }
    if (neos) {
        std::cerr << "Aux::TensorPacker: " << neos << " input tensors missing\n";
    }

    if (invec.size() != m_multiplicity) {
        std::cerr << "Aux::TensorPacker: got unexpected multiplicity, got:" << invec.size()
                  << " want:" << m_multiplicity << std::endl;
        THROW(ValueError() << errmsg{"unexpected multiplicity"});
    }

    ITensor::vector* itv = new ITensor::vector;
    for (auto iten : invec) {
        itv->push_back(iten);
        log->trace("tag: {}, type: {}", iten->metadata()["tag"], iten->metadata()["type"]);
    }

    // TODO: set md and ident
    Configuration set_md;
    out = std::make_shared<SimpleTensorSet>(0, set_md, ITensor::shared_vector(itv));

    return true;
}
