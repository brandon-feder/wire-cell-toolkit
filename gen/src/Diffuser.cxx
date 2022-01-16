#include "WireCellGen/Diffuser.h"
#include "WireCellGen/Diffusion.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Persist.h"
#include <boost/format.hpp>
#include <cmath>

#include <sstream>
#include <iostream>

WIRECELL_FACTORY(Diffuser, WireCell::Diffuser, WireCell::IDiffuser, WireCell::IConfigurable)

using namespace std;  // debugging
using namespace WireCell;
using boost::format;

Diffuser::Diffuser(const Ray& pitch, double binsize_l, double time_offset, double origin_l, double DL, double DT,
                   double drift_velocity, double max_sigma_l, double nsigma)
  : m_pitch_origin(pitch.first)
  , m_pitch_direction(ray_unit(pitch))
  , m_time_offset(time_offset)
  , m_origin_l(origin_l)
  , m_origin_t(0.0)  // measure pitch direction from pitch_origin
  , m_binsize_l(binsize_l)
  , m_binsize_t(ray_length(pitch))
  , m_DL(DL)
  , m_DT(DT)
  , m_drift_velocity(drift_velocity)
  , m_max_sigma_l(max_sigma_l)
  , m_nsigma(nsigma)
  , m_eos(false)
{
    // dump("Diffuser created");
}

Diffuser::~Diffuser() {}

Configuration Diffuser::default_configuration() const
{
    stringstream ss;
    ss << "{\n"
       << "\"pitch_origin\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\n"
       << "\"pitch_direction\":{\"x\":0.0,\"y\":0.0,\"z\":1.0},\n"
       << "\"pitch_distance\":" << 5.0 * units::mm << ",\n"
       << "\"timeslice\":" << 2.0 * units::us << ",\n"
       << "\"timeoffset\":0.0,\n"
       << "\"starttime\":0.0,\n"
       << "\"origin\":0.0,\n"
       << "\"DL\":" << 5.3 * units::centimeter2 / units::second << ",\n"
       << "\"DT\":" << 12.8 * units::centimeter2 / units::second << ",\n"
       << "\"drift_velocity\":" << 1.6 * units::mm / units::us << ",\n"
       << "\"max_sigma_l\":" << 5.0 * units::us << ",\n"
       << "\"nsigma\":3.0\n"
       << "}\n";
    return Persist::loads(ss.str());
}

void Diffuser::configure(const Configuration& cfg)
{
    m_pitch_origin = get<Point>(cfg, "pitch_origin", m_pitch_origin);
    m_pitch_direction = get<Point>(cfg, "pitch_direction", m_pitch_direction).norm();
    m_time_offset = get<double>(cfg, "timeoffset", m_time_offset);

    m_origin_l = get<double>(cfg, "starttime", m_origin_l);
    m_origin_t = get<double>(cfg, "origin", m_origin_t);

    m_binsize_l = get<double>(cfg, "timeslice", m_binsize_l);
    m_binsize_t = get<double>(cfg, "pitch_distance", m_binsize_t);

    m_DL = get<double>(cfg, "DL", m_DL);
    m_DT = get<double>(cfg, "DT", m_DT);

    m_drift_velocity = get<double>(cfg, "drift_velocity", m_drift_velocity);

    m_max_sigma_l = get<double>(cfg, "max_sigma_l", m_max_sigma_l);
    m_nsigma = get<double>(cfg, "nsigma", m_nsigma);

    // dump("Diffuser configured");
}

void Diffuser::reset() { m_input.clear(); }

void Diffuser::dump(const std::string& msg)
{
    stringstream ss;
    ss << msg << endl
       << "\tpitch origin " << m_pitch_origin << endl
       << "\tpitch direction " << m_pitch_direction << endl
       << "\ttime offset " << m_time_offset << endl
       << "\torigin l " << m_origin_l << endl
       << "\torigin t " << m_origin_t << endl
       << "\tbinsize l " << m_binsize_l << endl
       << "\tbinsize t " << m_binsize_t << endl
       << "\tDL = " << m_DL << " DT = " << m_DT << endl
       << "\tdrift velocity = " << m_drift_velocity << endl
       << "\tmax sigma l = " << m_max_sigma_l << endl
       << "\tnsigma = " << m_nsigma;
    cerr << ss.str() << endl;
    ;
}

bool Diffuser::operator()(const input_pointer& depo, output_queue& outq)
{
    if (m_eos) {
        return false;
    }
    if (!depo) {  // EOS flush
        for (auto diff : m_input) {
            outq.push_back(diff);
        }
        outq.push_back(nullptr);
        m_eos = true;
        return true;
    }

    auto first = *depo_chain(depo).rbegin();
    const double drift_distance = first->pos().x() - depo->pos().x();
    const double drift_time = drift_distance / m_drift_velocity;

    const double tmpcm2 = 2 * m_DL * drift_time / units::centimeter2;
    const double sigmaL = sqrt(tmpcm2) * units::centimeter / m_drift_velocity;
    const double sigmaT = sqrt(2 * m_DT * drift_time / units::centimeter2) * units::centimeter2;

    const Vector to_depo = depo->pos() - m_pitch_origin;
    const double pitch_distance = m_pitch_direction.dot(to_depo);

    cerr << "Diffuser: "
         << " drift distance=" << drift_distance << " drift time=" << drift_time
         << " pitch distance = " << pitch_distance << endl;

    IDiffusion::pointer diff =
        this->diffuse(m_time_offset + depo->time(), pitch_distance, sigmaL, sigmaT, depo->charge(), depo);
    m_input.insert(diff);

    while (m_input.size() > 2) {
        auto first = *m_input.begin();
        auto last = *m_input.rbegin();
        const double last_center = 0.5 * (last->lend() + last->lbegin());
        if (last_center - first->lbegin() < m_max_sigma_l * m_nsigma) {
            break;  // new input with long diffusion may still take lead
        }

        // now we are guaranteed no newly added diffusion can have a
        // leading edge long enough to surpass that of the current
        // leader.
        outq.push_back(first);
        m_input.erase(first);
    }

    return true;
}

std::vector<double> Diffuser::oned(double mean, double sigma, double binsize, const Diffuser::bounds_type& bounds)
{
    int nbins = round((bounds.second - bounds.first) / binsize);

    /// fragment between bin_edge_{low,high}
    std::vector<double> integral(nbins + 1, 0.0);
    for (int ibin = 0; ibin <= nbins; ++ibin) {
        double absx = bounds.first + ibin * binsize;
        double t = 0.5 * (absx - mean) / sigma;
        double e = 0.5 * std::erf(t);
        integral[ibin] = e;
    }

    std::vector<double> bins;
    for (int ibin = 0; ibin < nbins; ++ibin) {
        bins.push_back(integral[ibin + 1] - integral[ibin]);
    }
    return bins;
}

Diffuser::bounds_type Diffuser::bounds(double mean, double sigma, double binsize, double origin)
{
    double low = floor((mean - m_nsigma * sigma - origin) / binsize) * binsize + origin;
    double high = ceil((mean + m_nsigma * sigma - origin) / binsize) * binsize + origin;

    return std::make_pair(low, high);
}

IDiffusion::pointer Diffuser::diffuse(double mean_l, double mean_t, double sigma_l, double sigma_t, double weight,
                                      IDepo::pointer depo)
{
    bounds_type bounds_l = bounds(mean_l, sigma_l, m_binsize_l, m_origin_l);
    bounds_type bounds_t = bounds(mean_t, sigma_t, m_binsize_t, m_origin_t);

    std::vector<double> l_bins = oned(mean_l, sigma_l, m_binsize_l, bounds_l);
    std::vector<double> t_bins = oned(mean_t, sigma_t, m_binsize_t, bounds_t);

    if (l_bins.empty() || t_bins.empty()) {
        return nullptr;
    }

    // get normalization
    double power = 0;
    for (auto l : l_bins) {
        for (auto t : t_bins) {
            power += l * t;
        }
    }
    if (power == 0.0) {
        return nullptr;
    }

    Diffusion* smear = new Diffusion(depo, l_bins.size(), t_bins.size(), bounds_l.first, bounds_t.first,
                                     bounds_l.second, bounds_t.second);

    for (size_t ind_l = 0; ind_l < l_bins.size(); ++ind_l) {
        for (size_t ind_t = 0; ind_t < t_bins.size(); ++ind_t) {
            double value = l_bins[ind_l] * t_bins[ind_t] / power * weight;
            smear->set(ind_l, ind_t, value);
        }
    }

    IDiffusion::pointer ret(smear);
    return ret;
}
