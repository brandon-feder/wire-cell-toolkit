#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellIface/SimpleDepo.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Units.h"

#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IConfigurable.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TPolyMarker.h"

#include <iostream>

using namespace WireCell;
using namespace std;

struct Meta {
    // TApplication* theApp = 0;

    TCanvas* canvas;
    ExecMon em;
    const char* name;

    Meta(const char* name)
      //: theApp(new TApplication (name,0,0))
      : canvas(new TCanvas("canvas", "canvas", 500, 500))
      , em(name)
      , name(name)
    {
        print("[");
    }

    void print(const char* extra = "")
    {
        string fname = Form("%s.pdf%s", name, extra);
        // cerr << "Printing: " << fname << endl;
        canvas->Print(fname.c_str(), "pdf");
    }
};

const double t0 = 1 * units::s;  // put it way out in left field to catch any offset errors
// const double t0 = 0*units::s;
const int nticks = 9600;
const double tick = 0.5 * units::us;
const double readout_time = nticks * tick;
const double drift_speed = 1.6 * units::mm / units::us;

const int nwires = 2001;
const double wire_pitch = 3 * units::mm;
const double min_wire_pitch = -0.5 * (nwires - 1) * wire_pitch;
const double max_wire_pitch = +0.5 * (nwires - 1) * wire_pitch;

Pimpos pimpos(nwires, min_wire_pitch, max_wire_pitch);
Binning tbins(nticks, t0, t0 + readout_time);

// +/- this number of wire regions around wire of interest to assume
// we have field responses calculated.
const int npmwires = 10;

void test_track(Meta& meta, double charge, double track_time, const Ray& track_ray, double stepsize,
                IRandom::pointer fluctuate)
{
    const int ndiffision_sigma = 3.0;

    const auto rbins = pimpos.region_binning();
    const auto ibins = pimpos.impact_binning();

    Gen::BinnedDiffusion bd(pimpos, tbins, ndiffision_sigma, fluctuate);

    auto track_start = track_ray.first;
    auto track_dir = ray_unit(track_ray);
    auto track_length = ray_length(track_ray);

    const double DL = 5.3 * units::centimeter2 / units::second;
    const double DT = 12.8 * units::centimeter2 / units::second;

    const double xstop = pimpos.origin()[0];

    meta.em("begin adding Depp's");
    for (double dist = 0.0; dist < track_length; dist += stepsize) {
        auto pt = track_start + dist * track_dir;
        const double delta_x = pt.x() - xstop;
        const double drift_time = delta_x / drift_speed;
        pt.x(xstop);  // insta-drift
        const double time = track_time + drift_time;

        const double pitch = pimpos.distance(pt);
        Assert(rbins.inside(pitch));
        Assert(ibins.inside(pitch));  // should be identical
        Assert(tbins.inside(time));

        const double tmpcm2 = 2 * DL * drift_time / units::centimeter2;
        const double sigmaL = sqrt(tmpcm2) * units::centimeter / drift_speed;
        const double sigmaT = sqrt(2 * DT * drift_time / units::centimeter2) * units::centimeter2;

        auto depo = std::make_shared<SimpleDepo>(time, pt, charge);
        bd.add(depo, sigmaL, sigmaT);
        // cerr << "dist: " <<dist/units::mm << "mm, drift: " << drift_time/units::us << "us depo:" << depo->pos() << "
        // @ " << depo->time()/units::us << "us\n";
    }

    meta.em("begin swiping wires");

    for (int iwire = 0; iwire < nwires; ++iwire) {
        const int min_wire = std::max(iwire - npmwires, 0);
        const int max_wire = std::min(iwire + npmwires, nwires - 1);

        const int min_impact = std::max(pimpos.wire_impacts(min_wire).first, 0);
        const int max_impact = std::min(pimpos.wire_impacts(max_wire).second, ibins.nbins());

        std::pair<double, double> time_span(0, 0);
        std::vector<Gen::ImpactData::pointer> collect;

        std::set<int> seen;
        for (int imp = min_impact; imp <= max_impact; ++imp) {
            auto impact_data = bd.impact_data(imp);
            if (!impact_data) {
                continue;
            }

            const int impnum = impact_data->impact_number();
            if (seen.find(impnum) != seen.end()) {
                cerr << "Got duplicate impact number: " << impnum << " in wire " << iwire << endl;
            }
            seen.insert(impnum);
            if (iwire == 974) {
                cerr << "collecting: " << iwire << " " << impnum << endl;
            }

            auto ts = impact_data->span(ndiffision_sigma);
            if (collect.empty()) {
                time_span = ts;
            }
            else {
                time_span.first = std::min(time_span.first, ts.first);
                time_span.second = std::max(time_span.second, ts.second);
            }
            collect.push_back(impact_data);
        }

        if (collect.empty()) {
            continue;
        }

        // bd.erase(0, min_impact);

        if (false) {
            cerr << "Not histogramming\n";
            continue;
        }

        const int min_tedge = tbins.edge_index(time_span.first);
        const int max_tedge = tbins.edge_index(time_span.second);

        const double min_time = tbins.edge(min_tedge);
        const double max_time = tbins.edge(max_tedge);
        const int ntbins = max_tedge - min_tedge;

        const double min_pitch = ibins.edge(min_impact);
        const double max_pitch = ibins.edge(max_impact);
        const int npbins = (max_pitch - min_pitch) / ibins.binsize();

        // cerr << "t:"<<ntbins<<"["<<min_time/units::us<<","<<max_time/units::us<<"]us ("<<max_time-min_time<<")\n";
        // cerr << "p:"<<npbins<<"["<<min_pitch/units::mm<<","<<max_pitch/units::mm<<"]mm\n";

        Assert(max_time > min_time);
        Assert(ntbins > 1);
        Assert(max_pitch > min_pitch);
        Assert(npbins > 1);

        TH2F hist(Form("hwire%04d", iwire), Form("Diffused charge for wire %d", iwire), ntbins,
                  (min_time - t0) / units::us, (max_time - t0) / units::us, npbins, min_pitch / units::mm,
                  max_pitch / units::mm);
        hist.SetXTitle("time (us)");
        hist.SetYTitle("pitch (mm)");

        TH1F hp(Form("pwire%04d", iwire), Form("Pitches for wire %d", iwire), npbins, min_pitch / units::mm,
                max_pitch / units::mm);
        TH1F ht(Form("twire%04d", iwire), Form("Times for wire %d", iwire), ntbins, (min_time - t0) / units::us,
                (max_time - t0) / units::us);

        for (auto idptr : collect) {
            auto wave = idptr->waveform();
            Assert(wave.size() == nticks);
            const int impact = idptr->impact_number();
            const double pitch_dist = ibins.center(impact);
            if (iwire == 974) {
                cerr << iwire << " impact=" << impact << " pitch=" << pitch_dist << endl;
            }
            auto mm = idptr->span(ndiffision_sigma);
            const int min_tick = tbins.bin(mm.first);
            const int max_tick = tbins.bin(mm.second);
            // cerr << "impact:" << impact << " pitch="<<pitch_dist/units::mm << "
            // ticks:["<<min_tick<<","<<max_tick<<"]\n";
            for (int itick = min_tick; itick <= max_tick; ++itick) {
                const double time = tbins.center(itick);
                if (!tbins.inside(time)) {
                    cerr << "OOB time: " << time / units::us << "us tick:" << itick
                         << " impact:" << idptr->impact_number() << " pitch:" << pitch_dist / units::mm << "mm\n";
                }

                Assert(tbins.inside(time));
                Assert(rbins.inside(pitch_dist));
                const double t_us = (time - t0) / units::us;
                const double p_mm = pitch_dist / units::mm;
                hist.Fill(t_us, p_mm, wave[itick]);
                ht.Fill(t_us);
                hp.Fill(p_mm);
            }
        }
        hist.Draw("colz");
        // hp.Draw();
        hist.Write();
        ht.Write();
        hp.Write();
        meta.print();
    }
    meta.em("done");
}

int main(int argc, char* argv[])
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    {
        auto rngcfg = Factory::lookup<IConfigurable>("Random");
        auto cfg = rngcfg->default_configuration();
        rngcfg->configure(cfg);
    }
    auto rng = Factory::lookup<IRandom>("Random");

    const char* me = argv[0];

    TFile* rootfile = TFile::Open(Form("%s.root", me), "RECREATE");

    Meta meta(me);
    gStyle->SetOptStat(0);

    const double track_time = t0 + 10 * units::ns;
    const double delta = 100 * units::mm;
    Ray track_ray(Point(1 * units::m - delta, 0, -delta), Point(1 * units::m + delta, 0, +delta));
    const double stepsize = 1 * units::mm;
    const double charge = 1e5;

    test_track(meta, charge, track_time, track_ray, stepsize, rng);

    meta.print("]");
    rootfile->Close();
    cerr << meta.em.summary() << endl;
    return 0;
}
