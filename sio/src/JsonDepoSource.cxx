#include "WireCellSio/JsonDepoSource.h"

#include "WireCellIface/IRecombinationModel.h"

#include "WireCellIface/SimpleDepo.h"
#include "WireCellUtil/NamedFactory.h"

#include "WireCellUtil/Point.h"
#include "WireCellUtil/Persist.h"

WIRECELL_FACTORY(JsonDepoSource, WireCell::Sio::JsonDepoSource, WireCell::IDepoSource, WireCell::IConfigurable)

#include <iostream>
#include <string>
#include <locale>  // for std::tolower

using namespace WireCell;

// These adapters deal with whether the file holds just "n" number of
// electrons, just "q" at a point or both "q" and "s" along a step.
// For the first, just a units conversion is needed.  In the latter
// two cases a real RecombinationModel is used to return amount of
// drifting charge for given point energy deposition "q" or as both
// q=dE and s=dX.
class Sio::JsonRecombinationAdaptor {
   public:
    virtual ~JsonRecombinationAdaptor() {}
    virtual double operator()(Json::Value depo) const = 0;
};
class ElectronsAdapter : public Sio::JsonRecombinationAdaptor {
    double m_scale;

   public:
    ElectronsAdapter(double scale = 1.0)
      : m_scale(scale)
    {
    }
    virtual ~ElectronsAdapter(){};
    virtual double operator()(Json::Value depo) const { return m_scale * depo["n"].asInt() * (-1.0 * units::eplus); }
};
class PointAdapter : public Sio::JsonRecombinationAdaptor {
    IRecombinationModel::pointer m_model;

   public:
    PointAdapter(IRecombinationModel::pointer model)
      : m_model(model)
    {
    }
    virtual ~PointAdapter() {}
    virtual double operator()(Json::Value depo) const
    {
        const double dE = depo["q"].asDouble();
        return (*m_model)(dE);
    }
};
class StepAdapter : public Sio::JsonRecombinationAdaptor {
    IRecombinationModel::pointer m_model;

   public:
    StepAdapter(IRecombinationModel::pointer model)
      : m_model(model)
    {
    }
    virtual ~StepAdapter() {}
    virtual double operator()(Json::Value depo) const
    {
        const double dE = depo["q"].asDouble();
        const double dX = depo["s"].asDouble();
        return (*m_model)(dE, dX);
    }
};

using namespace std;
using namespace WireCell;

Sio::JsonDepoSource::JsonDepoSource()
  : m_adapter(nullptr)
  , m_eos(false)
{
}

Sio::JsonDepoSource::~JsonDepoSource() {}

bool Sio::JsonDepoSource::operator()(IDepo::pointer& out)
{
    if (m_eos) {
        return false;
    }

    if (m_depos.empty()) {
        m_eos = true;
        out = nullptr;
        return true;
    }

    out = m_depos.back();
    m_depos.pop_back();
    return true;
}

WireCell::Configuration Sio::JsonDepoSource::default_configuration() const
{
    Configuration cfg;
    cfg["filename"] = "";        // json file name
    cfg["jsonpath"] = "depos";   // path to depo list in json data
    cfg["model"] = "electrons";  // model for converting "q" and maybe
                                 // "s" or "n" to amount of drifting
                                 // charge.
    return cfg;
}

IDepo::pointer Sio::JsonDepoSource::jdepo2idepo(Json::Value jdepo)
{
    const double q = (*m_adapter)(jdepo);
    auto idepo = std::make_shared<SimpleDepo>(
        get(jdepo, "t", 0.0), Point(get(jdepo, "x", 0.0), get(jdepo, "y", 0.0), get(jdepo, "z", 0.0)), q);
    return idepo;
}

void Sio::JsonDepoSource::configure(const WireCell::Configuration& cfg)
{
    m_depos.clear();
    if (m_adapter) {
        delete m_adapter;
        m_adapter = nullptr;
    }
    std::string model_tn = cfg["model"].asString();
    std::string model_type = String::split(model_tn)[0];

    if (model_type == "electrons") {  // "n" already gives number of ionization electrons
        double scale = get(cfg, "scale", 1.0);
        cerr << "Sio::JsonDepoSource: using electrons with scale=" << scale << endl;
        m_adapter = new ElectronsAdapter(scale);
    }
    else {
        auto model = Factory::lookup_tn<IRecombinationModel>(model_tn);
        if (!model) {
            cerr << "Sio::JsonDepoSource::configure: unknown recombination model: \"" << model_tn << "\"\n";
            return;
        }
        if (model_type == "MipRecombination") {
            m_adapter = new PointAdapter(model);
        }
        if (model_type == "BirksRecombination" || model_type == "BoxRecombination") {
            m_adapter = new StepAdapter(model);
        }
    }

    // get and load JSON file.
    string filename = get<string>(cfg, "filename");
    string dotpath = get<string>(cfg, "jsonpath", "depos");
    if (filename.empty()) {
        cerr << "JsonDepoSource::configure: no JSON filename given" << endl;
        return;  // fixme: uh, error handle much?
    }
    Json::Value top = WireCell::Persist::load(filename.c_str());

    double qtot = 0;
    auto jdepos = branch(top, dotpath);
    for (auto jdepo : jdepos) {
        auto idepo = jdepo2idepo(jdepo);
        m_depos.push_back(idepo);
        qtot += idepo->charge();
    }
    std::sort(m_depos.begin(), m_depos.end(), descending_time);
    cerr << "Sio::JsonDepoSource::configure: "
         << "slurped in " << m_depos.size() << " depositions, "
         << " = " << -1 * qtot / units::eplus << " electrons\n";
}
