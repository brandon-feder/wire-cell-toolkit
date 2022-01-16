#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDepoSource.h"

#include "TFile.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2F.h"

#include <iostream>

#include "ar39.h"

using namespace WireCell;
using namespace WireCell::Gen::Test;

int main(int argc, char* argv[])
{
    ExecMon em("start");
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");

    const int nebins = 100;
    const int specsize = ar39_mev.size();
    const double enemin = ar39_mev.front();
    const double enemax = ar39_mev.back();
    const double enebin = (enemax - enemin) / specsize;

    // note: this all assumes something close to a MIP dE/dX
    // for the full story see: http://lar.bnl.gov/properties/#particle-pass
    // 0.7 * 1 MeV / 23.6 eV = 29661 electrons at 500 V/cm
    // 0.64* 1 MeV / 23.6 eV = 27118 electrons at 300 V/cm
    const double ele_per_mev = 28000;  // ballpark

    TGraph spectrum;
    double spectot = 0.0;

    // Normal user component code should never do this
    {
        auto rng_cfg = Factory::lookup<IConfigurable>("Random");
        {
            auto cfg = rng_cfg->default_configuration();
            rng_cfg->configure(cfg);
        }
        auto bs_cfg = Factory::lookup<IConfigurable>("BlipSource");
        {
            auto cfg = bs_cfg->default_configuration();
            Configuration edges, pdf, nele;
            for (int ind = 0; ind < specsize; ++ind) {
                const double e = ar39_mev[ind];
                const double p = ar39_pdf[ind];
                edges[ind] = e;
                nele[ind] = e * ele_per_mev;
                pdf[ind] = p;
                spectot += p;
            }
            for (int ind = 0; ind < specsize; ++ind) {
                const double prob = ar39_pdf[ind] / (spectot * enebin);
                const double ene = ar39_mev[ind];
                spectrum.SetPoint(ind, ene, prob);
            }
            Configuration ene;
            ene["type"] = "pdf";
            ene["edges"] = edges;
            ene["pdf"] = pdf;
            cfg["charge"] = ene;
            // std::cout << cfg << std::endl;
            bs_cfg->configure(cfg);
            ene["edges"] = nele;

            // pos config is (2m)^2 box centered on origin

            std::cout << "Configuration for Ar39 blips:\n" << ene << std::endl;
        }
    }

    // Normal user component code might do this but should make the
    // name a configurable parameter.
    auto deposrc = Factory::lookup<IDepoSource>("BlipSource");

    em("initialized");

    TFile* rootfile = TFile::Open(Form("%s.root", argv[0]), "recreate");
    gStyle->SetOptStat(111111);
    gStyle->SetOptFit(111111);
    TCanvas canvas("canvas", "canvas", 1000, 800);

    auto hxy = new TH2F("hxy", "Decay location (Y vs X)", 200, -1.0, 1.0, 200, -1.0, 1.0);
    auto hxz = new TH2F("hxz", "Decay location (Z vs X)", 200, -1.0, 1.0, 200, -1.0, 1.0);

    auto ht = new TH1D("ht", "Ar39 decay time", 1000, 0, 100);
    ht->GetXaxis()->SetTitle("time between decays [us]");
    auto he = new TH1D("he", "Ar39 decay spectrum", nebins, 0, 1);  // fixme: this needs to change to electrons!
    he->GetXaxis()->SetTitle("beta kinetic energy (MeV)");
    auto hde = new TH1D("hde", "Energy bin size (round-off errors)", 200, 0.01 - 0.0001, 0.01 + 0.0001);
    {  // make sure we only see round-off errors
        double last_e = 0.0;
        for (const double e : ar39_mev) {
            hde->Fill(e - last_e + 0.00001);
            last_e = e;
        }
    }

    double last_time = 0.0;
    for (int ind = 0; ind != 10000000; ++ind) {
        IDepo::pointer depo;
        bool ok = (*deposrc)(depo);
        if (!ok) {
            std::cerr << "BlipSource failed!\n";
            return 1;
        }
        if (!depo) {
            std::cerr << "BlipSource: EOS\n";
            break;
        }
        const double charge = depo->charge();
        if (charge < 0) {
            std::cerr << "Got negative charge\n";
            return 1;
        }
        he->Fill(charge);
        const double time = depo->time();
        ht->Fill((time - last_time) / units::us);
        last_time = time;

        auto& pos = depo->pos();
        hxy->Fill(pos.x() / units::m, pos.y() / units::m, charge);
        hxz->Fill(pos.x() / units::m, pos.z() / units::m, charge);
    }
    he->Scale(1.0 / (he->Integral() / nebins));

    em("filled");

    std::string fname = Form("%s.pdf", argv[0]);
    canvas.Print((fname + "[").c_str(), "pdf");

    ht->Fit("expo");
    ht->Draw();
    canvas.Print(fname.c_str(), "pdf");
    em("fitted");

    spectrum.SetLineColor(2);
    he->Draw();
    spectrum.Draw("L");
    canvas.Print(fname.c_str(), "pdf");

    hde->Draw();
    canvas.Print(fname.c_str(), "pdf");

    hxy->Draw("colz");
    canvas.Print(fname.c_str(), "pdf");

    hxz->Draw("colz");
    canvas.Print(fname.c_str(), "pdf");

    canvas.Print((fname + "]").c_str(), "pdf");
    em("plotted");

    ht->Write();
    he->Write();
    hde->Write();
    rootfile->Close();

    em("done");
    std::cerr << em.summary() << std::endl;
    return 0;
}
