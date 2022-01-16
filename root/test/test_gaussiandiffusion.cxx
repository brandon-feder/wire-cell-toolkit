#include "WireCellGen/GaussianDiffusion.h"
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
#include "TPolyMarker.h"

#include <iostream>

using namespace WireCell;
using namespace std;

void test_binint()
{
    const double mean = 1.7;
    const double sigma = 2.0;
    const double xmin = -5;
    const double xmax = +5;

    const Gen::GausDesc gd(mean, sigma);
    const vector<int> nbinsv{100, 50, 20, 10, 5};
    const vector<int> color{1, 2, 4, 6, 7, 8};

    cerr << "erf(+1) = " << std::erf(+1.0) << endl;
    cerr << "erf(-1) = " << std::erf(-1.0) << endl;

    for (size_t ind = 0; ind < nbinsv.size(); ++ind) {
        const int nbins = nbinsv[ind];
        Binning b(nbins, xmin, xmax);
        auto bi = gd.binint(b.min(), b.binsize(), nbins);
        TH1F* h = new TH1F("h", "binint Gaussian per binsize", nbins, xmin, xmax);
        for (int bin = 0; bin < nbins; ++bin) {
            h->Fill(b.center(bin), bi[bin] / b.binsize());
        }
        const double integ = h->Integral();
        cerr << ind << " integ=" << b.binsize() * integ << " raw=" << integ << endl;
        h->SetLineColor(color[ind]);
        if (ind) {
            h->Draw("HIST,SAME");
        }
        else {
            h->Draw("HIST");
        }
    }
}

void test_gd(IRandom::pointer fluctuate)
{
    const double nsigma = 3.0;

    /// Time Gaussian
    const double t_center = 3 * units::ms;
    const double t_sigma = 1 * units::us;
    Gen::GausDesc tdesc(t_center, t_sigma);

    /// Pitch Gaussian
    const double p_center = 1 * units::m;
    const double p_sigma = 1 * units::mm;
    Gen::GausDesc pdesc(p_center, p_sigma);

    const double nsigma_binning = 2.0 * nsigma;
    /// Time bins
    const double t_sample = 0.5 * units::us;
    const double t_min = t_center - nsigma_binning * t_sigma;
    const double t_max = t_center + nsigma_binning * t_sigma;
    const int nt = (t_max - t_min) / t_sample;
    const Binning tbins(nt, t_min, t_max);

    /// Pitch bins
    const double p_sample = 0.3 * units::mm;
    const double p_min = p_center - nsigma_binning * p_sigma;
    const double p_max = p_center + nsigma_binning * p_sigma;
    const int np = (p_max - p_min) / p_sample;
    const Binning pbins(np, p_min, p_max);

    /// Make a single deposition
    const double qdepo = -1000.0;
    const Point pdepo(10 * units::cm, 0.0, p_center);
    auto depo = std::make_shared<SimpleDepo>(t_center, pdepo, qdepo);

    /// Note it is up to caller to assure that depo and tdesc/pdesc
    /// are consistent!  See BinnedDiffussion for one class that does
    /// this.
    Gen::GaussianDiffusion gd(depo, tdesc, pdesc);

    /// Rastering to an array is delayed
    cerr << "Set sampling: tbins=" << tbins << ", pbins=" << pbins << ", nsigma=" << nsigma
         << ", fluctuate:" << fluctuate << endl;
    gd.set_sampling(tbins, pbins, nsigma, fluctuate);

    /// patch only covers +/- nsigma
    auto patch = gd.patch();
    const int toffset = gd.toffset_bin();
    const int poffset = gd.poffset_bin();

    cerr << "rows=" << patch.rows() << " cols=" << patch.cols() << endl;
    cerr << "toffset=" << toffset << " poffset=" << poffset << endl;

    Assert(toffset >= 0);
    Assert(poffset >= 0);

    const double tunit = units::us;  // for display
    const double punit = units::mm;  // for display

    TPolyMarker* marker = new TPolyMarker(1);
    marker->SetPoint(0, t_center / tunit, p_center / punit);
    marker->SetMarkerStyle(5);
    cerr << "center t=" << t_center / tunit << ", p=" << p_center / punit << endl;

    TH2F* hist = new TH2F("patch1", "Diffusion Patch", tbins.nbins(), tbins.min() / tunit, tbins.max() / tunit,
                          pbins.nbins(), pbins.min() / punit, pbins.max() / punit);

    double total = 0.0;
    hist->SetXTitle("time (us)");
    hist->SetYTitle("pitch (mm)");
    for (int it = 0; it < patch.cols(); ++it) {
        double tval = tbins.center(it + toffset);
        Assert(tbins.inside(tval));
        for (int ip = 0; ip < patch.rows(); ++ip) {
            double pval = pbins.edge(ip + poffset) + 0.0001;
            Assert(pbins.inside(pval));
            const double value = patch(ip, it);
            hist->Fill(tval / tunit, pval / punit, value);
            total += value;
        }
    }
    hist->Write();
    hist->Draw("colz");
    marker->Draw();
    cerr << "total=" << total << " integ=" << hist->Integral() << " maximum=" << hist->GetMaximum() << endl;
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

    TApplication* theApp = 0;
    if (argc > 1) {
        theApp = new TApplication(me, 0, 0);
    }
    TFile output(Form("%s.root", me), "RECREATE");
    TCanvas canvas("canvas", "canvas", 500, 500);
    canvas.Print(Form("%s.pdf[", me), "pdf");
    gStyle->SetOptStat(0);

    test_binint();
    canvas.Print(Form("%s.pdf", me), "pdf");

    test_gd(nullptr);
    canvas.Print(Form("%s.pdf", me), "pdf");
    test_gd(rng);
    canvas.Print(Form("%s.pdf", me), "pdf");

    if (theApp) {
        theApp->Run();
    }
    else {  // batch
        canvas.Print(Form("%s.pdf]", me), "pdf");
    }

    return 0;
}
