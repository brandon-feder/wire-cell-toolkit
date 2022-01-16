#include "WireCellGen/BlipSource.h"
#include "WireCellIface/SimpleDepo.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

WIRECELL_FACTORY(BlipSource, WireCell::Gen::BlipSource, WireCell::IDepoSource, WireCell::IConfigurable)

using namespace WireCell;

Gen::BlipSource::BlipSource()
  : m_rng_tn("Random")
  , m_time(0.0)
  , m_ene(nullptr)
  , m_tim(nullptr)
  , m_pos(nullptr)
  , m_blip_count(0)
  , m_eos(false)
{
}

Gen::BlipSource::~BlipSource()
{
    delete m_ene;
    m_ene = nullptr;
    delete m_tim;
    m_tim = nullptr;
    delete m_pos;
    m_pos = nullptr;
}

WireCell::Configuration Gen::BlipSource::default_configuration() const
{
    Configuration cfg;
    cfg["rng"] = "Random";

    Configuration ene;
    ene["type"] = "mono";  // future: spectrum, uniform, Ar39
    ene["value"] = 20000.0;
    cfg["charge"] = ene;
    // also, in Jsonnet can support PDFs giving probability for certain number of electrons
    // { type="pdf",
    //   edges=[(0.1+b*0.1)*wc.MeV for b in std.range(0,10)], // energy edges
    //   pdf=[my_pdf_function(self.edges)]}

    Configuration tim;
    tim["type"] = "decay";
    tim["start"] = 0.0;
    tim["stop"] = 10.0 * units::ms;
    tim["activity"] = 100000 * units::Bq;  // Ar39 in 2 DUNE drift volumes
    cfg["time"] = tim;

    Configuration pos;
    pos["type"] = "box";
    Configuration tail, head, extent;
    tail["x"] = -1 * units::m;
    tail["y"] = -1 * units::m;
    tail["z"] = -1 * units::m;
    head["x"] = +1 * units::m;
    head["y"] = +1 * units::m;
    head["z"] = +1 * units::m;
    extent["tail"] = tail;
    extent["head"] = head;
    pos["extent"] = extent;

    cfg["position"] = pos;

    return cfg;
}

// Simply return the number given
struct ReturnValue : public Gen::BlipSource::ScalarMaker {
    double value;
    ReturnValue(double val)
      : value(val)
    {
    }
    ~ReturnValue() {}
    double operator()() { return value; }
};
// Choose scalar value from an exponential distribution
struct DecayTime : public Gen::BlipSource::ScalarMaker {
    IRandom::pointer rng;
    double rate;
    DecayTime(IRandom::pointer rng, double rate)
      : rng(rng)
      , rate(rate)
    {
    }
    ~DecayTime() {}
    double operator()() { return rng->exponential(rate); }
};
// draw from a PDF
struct Pdf : public Gen::BlipSource::ScalarMaker {
    IRandom::pointer rng;
    std::vector<double> cumulative;
    std::vector<double> edges;
    double total;
    Pdf(IRandom::pointer rng, const std::vector<double>& pdf, const std::vector<double>& edges)
      : rng(rng)
      , edges(edges.begin(), edges.end())
    {
        total = 0.0;
        cumulative.push_back(0.0);
        for (double val : pdf) {
            total += val;
            cumulative.push_back(total);
        }
    }
    double operator()()
    {
        double cp = rng->uniform(0.0, total);
        for (size_t ind = 1; ind < cumulative.size(); ++ind) {  // start at 2nd element
            if (cumulative[ind] >= cp) {
                double rel = (cp - cumulative[ind - 1]) / (cumulative[ind] - cumulative[ind - 1]);
                return edges[ind - 1] + rel * (edges[ind] - edges[ind - 1]);
            }
        }
        return -1;  // should not happen
    }
};

// return point selected uniformly from some box
struct UniformBox : public Gen::BlipSource::PointMaker {
    IRandom::pointer rng;
    Ray extent;
    UniformBox(IRandom::pointer rng, const Ray& extent)
      : rng(rng)
      , extent(extent)
    {
    }
    ~UniformBox() {}
    Point operator()()
    {
        auto p =
            Point(rng->uniform(extent.first.x(), extent.second.x()), rng->uniform(extent.first.y(), extent.second.y()),
                  rng->uniform(extent.first.z(), extent.second.z()));
        // std::cerr << "UniformBox: pt=" << p << std::endl;
        return p;
    }
};

void Gen::BlipSource::configure(const WireCell::Configuration& cfg)
{
    m_rng_tn = get(cfg, "rng", m_rng_tn);
    m_rng = Factory::find_tn<IRandom>(m_rng_tn);

    auto ene = cfg["charge"];
    if (ene["type"].asString() == "mono") {
        m_ene = new ReturnValue(ene["value"].asDouble());
    }
    else if (ene["type"].asString() == "pdf") {
        m_ene = new Pdf(m_rng, get<std::vector<double> >(ene, "pdf"), get<std::vector<double> >(ene, "edges"));
    }
    else {
        std::cerr << "BlipSource: no charge configuration\n";
        THROW(ValueError() << errmsg{"BlipSource: no charge configuration"});
    }

    auto tim = cfg["time"];
    m_time = tim["start"].asDouble();
    m_stop = tim["stop"].asDouble();
    if (tim["type"].asString() == "decay") {
        m_tim = new DecayTime(m_rng, tim["activity"].asDouble());
    }
    else {
        std::cerr << "BlipSource: no time configuration\n";
        THROW(ValueError() << errmsg{"BlipSource: no time configuration"});
    }

    auto pos = cfg["position"];
    if (pos["type"].asString() == "box") {
        Ray box = WireCell::convert<Ray>(pos["extent"]);
        std::cerr << "Box: \n\t" << box.first / units::mm << "mm\n\t" << box.second / units::mm << "mm\n";
        m_pos = new UniformBox(m_rng, box);
    }
    else {
        std::cerr << "BlipSource: no position configuration\n";
        THROW(ValueError() << errmsg{"BlipSource: no position configuration"});
    }
}

bool Gen::BlipSource::operator()(IDepo::pointer& depo)
{
    if (m_eos) {
        return false;
    }

    m_time += (*m_tim)();
    if (m_time > m_stop) {
        std::cerr << "BlipSource: reached stop time: " << m_time / units::ms << " > " << m_stop / units::ms
                  << std::endl;
        depo = nullptr;
        m_eos = true;
        return true;
    }
    ++m_blip_count;
    depo = std::make_shared<SimpleDepo>(m_time, (*m_pos)(), (*m_ene)(), nullptr, 0, 0, m_blip_count);
    return true;
}
