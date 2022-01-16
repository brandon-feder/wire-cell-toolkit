#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellGen/GaussianDiffusion.h"
#include "WireCellUtil/Units.h"

#include <iostream>  // debug
using namespace std;

using namespace WireCell;

Gen::BinnedDiffusion::BinnedDiffusion(const Pimpos& pimpos, const Binning& tbins, double nsigma,
                                      IRandom::pointer fluctuate, ImpactDataCalculationStrategy calcstrat)
  : m_pimpos(pimpos)
  , m_tbins(tbins)
  , m_nsigma(nsigma)
  , m_fluctuate(fluctuate)
  , m_calcstrat(calcstrat)
  , m_window(0, 0)
  , m_outside_pitch(0)
  , m_outside_time(0)
{
}

bool Gen::BinnedDiffusion::add(IDepo::pointer depo, double sigma_time, double sigma_pitch)
{
    const double center_time = depo->time();
    const double center_pitch = m_pimpos.distance(depo->pos());

    Gen::GausDesc time_desc(center_time, sigma_time);
    {
        double nmin_sigma = time_desc.distance(m_tbins.min());
        double nmax_sigma = time_desc.distance(m_tbins.max());

        double eff_nsigma = sigma_time > 0 ? m_nsigma : 0;
        if (nmin_sigma > eff_nsigma || nmax_sigma < -eff_nsigma) {
            // std::cerr << "BinnedDiffusion: depo too far away in time sigma:"
            //           << " t_depo=" << center_time/units::ms << "ms not in:"
            //           << " t_bounds=[" << m_tbins.min()/units::ms << ","
            //           << m_tbins.max()/units::ms << "]ms"
            //           << " in Nsigma: [" << nmin_sigma << "," << nmax_sigma << "]\n";
            ++m_outside_time;
            return false;
        }
    }

    auto ibins = m_pimpos.impact_binning();

    Gen::GausDesc pitch_desc(center_pitch, sigma_pitch);
    {
        double nmin_sigma = pitch_desc.distance(ibins.min());
        double nmax_sigma = pitch_desc.distance(ibins.max());

        double eff_nsigma = sigma_pitch > 0 ? m_nsigma : 0;
        if (nmin_sigma > eff_nsigma || nmax_sigma < -eff_nsigma) {
            // std::cerr << "BinnedDiffusion: depo too far away in pitch sigma: "
            //           << " p_depo=" << center_pitch/units::cm << "cm not in:"
            //           << " p_bounds=[" << ibins.min()/units::cm << ","
            //           << ibins.max()/units::cm << "]cm"
            //           << " in Nsigma:[" << nmin_sigma << "," << nmax_sigma << "]\n";
            ++m_outside_pitch;
            return false;
        }
    }

    // make GD and add to all covered impacts
    int bin_beg = std::max(ibins.bin(center_pitch - sigma_pitch * m_nsigma), 0);
    int bin_end = std::min(ibins.bin(center_pitch + sigma_pitch * m_nsigma) + 1, ibins.nbins());
    // debug
    // int bin_center = ibins.bin(center_pitch);
    // cerr << "DEBUG center_pitch: "<<center_pitch/units::cm<<endl;
    // cerr << "DEBUG bin_center: "<<bin_center<<endl;

    auto gd = std::make_shared<GaussianDiffusion>(depo, time_desc, pitch_desc);
    for (int bin = bin_beg; bin < bin_end; ++bin) {
        //   if (bin == bin_beg)  m_diffs.insert(gd);
        this->add(gd, bin);
    }

    return true;
}

void Gen::BinnedDiffusion::add(std::shared_ptr<GaussianDiffusion> gd, int bin)
{
    ImpactData::mutable_pointer idptr = nullptr;
    auto it = m_impacts.find(bin);
    if (it == m_impacts.end()) {
        idptr = std::make_shared<ImpactData>(bin);
        m_impacts[bin] = idptr;
    }
    else {
        idptr = it->second;
    }
    idptr->add(gd);
    if (false) {  // debug
        auto mm = idptr->span();
        cerr << "Gen::BinnedDiffusion: add: "
             << " poffoset=" << gd->poffset_bin() << " toffoset=" << gd->toffset_bin()
             << " charge=" << gd->depo()->charge() / units::eplus << " eles"
             << ", for bin " << bin << " t=[" << mm.first / units::us << "," << mm.second / units::us << "]us\n";
    }
    m_diffs.insert(gd);
    // m_diffs.push_back(gd);
}

void Gen::BinnedDiffusion::erase(int begin_impact_number, int end_impact_number)
{
    for (int bin = begin_impact_number; bin < end_impact_number; ++bin) {
        m_impacts.erase(bin);
    }
}

Gen::ImpactData::pointer Gen::BinnedDiffusion::impact_data(int bin) const
{
    const auto ib = m_pimpos.impact_binning();
    if (!ib.inbounds(bin)) {
        return nullptr;
    }

    auto it = m_impacts.find(bin);
    if (it == m_impacts.end()) {
        return nullptr;
    }
    auto idptr = it->second;

    // make sure all diffusions have been sampled
    for (auto diff : idptr->diffusions()) {
        diff->set_sampling(m_tbins, ib, m_nsigma, m_fluctuate, m_calcstrat);
        // diff->set_sampling(m_tbins, ib, m_nsigma, 0, m_calcstrat);
    }

    idptr->calculate(m_tbins.nbins());
    return idptr;
}

static std::pair<double, double> gausdesc_range(const std::vector<Gen::GausDesc> gds, double nsigma)
{
    int ncount = -1;
    double vmin = 0, vmax = 0;
    for (auto gd : gds) {
        ++ncount;

        const double lvmin = gd.center - gd.sigma * nsigma;
        const double lvmax = gd.center + gd.sigma * nsigma;
        if (!ncount) {
            vmin = lvmin;
            vmax = lvmax;
            continue;
        }
        vmin = std::min(vmin, lvmin);
        vmax = std::max(vmax, lvmax);
    }
    return std::make_pair(vmin, vmax);
}

std::pair<double, double> Gen::BinnedDiffusion::pitch_range(double nsigma) const
{
    std::vector<Gen::GausDesc> gds;
    for (auto diff : m_diffs) {
        gds.push_back(diff->pitch_desc());
    }
    return gausdesc_range(gds, nsigma);
}

std::pair<int, int> Gen::BinnedDiffusion::impact_bin_range(double nsigma) const
{
    const auto ibins = m_pimpos.impact_binning();
    auto mm = pitch_range(nsigma);
    return std::make_pair(std::max(ibins.bin(mm.first), 0), std::min(ibins.bin(mm.second) + 1, ibins.nbins()));
}

std::pair<double, double> Gen::BinnedDiffusion::time_range(double nsigma) const
{
    std::vector<Gen::GausDesc> gds;
    for (auto diff : m_diffs) {
        gds.push_back(diff->time_desc());
    }
    return gausdesc_range(gds, nsigma);
}

std::pair<int, int> Gen::BinnedDiffusion::time_bin_range(double nsigma) const
{
    auto mm = time_range(nsigma);
    return std::make_pair(std::max(m_tbins.bin(mm.first), 0), std::min(m_tbins.bin(mm.second) + 1, m_tbins.nbins()));
}
