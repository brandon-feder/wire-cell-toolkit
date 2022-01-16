/** Simple testing of field response interpolation
 *  Linear weighting/re-distributing charge instead
 *  Implementation in GaussianDiffusion for each charge depo
 */
#include "WireCellGen/GaussianDiffusion.h"
#include "WireCellGen/ImpactZipper.h"
#include "WireCellGen/TrackDepos.h"
#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellGen/TransportedDepo.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Testing.h"

#include "WireCellIface/IRandom.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TLine.h"
#include "TStyle.h"
#include "TH2F.h"
#include "TMath.h"
#include "TGraph.h"
#include "TBox.h"
#include "TF1.h"

#include <iostream>
#include <string>

using namespace WireCell;
using namespace std;

// linear weighting for the charge in each bin hornoring a Gaussian distribution
// cross check on WireCell::Gen::GaussianDiffusion
double WeightGaus(double x1, double x2, double mean, double sigma)
{
    double val = 0;
    if (!sigma) {
        val = mean - x2;
    }
    else {
        Double_t sqrt2 = TMath::Sqrt(2);
        val = TMath::Gaus(x2, mean, sigma, 1) - TMath::Gaus(x1, mean, sigma, 1);
        val *=
            -2. * sigma * sigma / (TMath::Erf((x2 - mean) / sqrt2 / sigma) - TMath::Erf((x1 - mean) / sqrt2 / sigma));
        val += (mean - x2);
    }
    return val / (x1 - x2);
}

int main(const int argc, char *argv[])
{
    string out_basename = argv[0];
    if (argc > 3) {
        out_basename = argv[3];
    }

    WireCell::ExecMon em(out_basename);

    // Generate trivial track of points
    const double stepsize = 0.01 * units::mm;
    Gen::TrackDepos tracks(stepsize);

    const double t0 = 0.0 * units::s;
    const double event_time = t0 + 1 * units::ms;
    const Point start_vertex(1 * units::m, 0 * units::m, 0.1 * units::mm);

    // 6 points inbetween an impact pitch
    for (int i = 0; i < 6; i++) {
        const Point vertex = start_vertex + Vector(0, 0, i * 0.06 * units::mm);
        tracks.add_track(event_time + i * 1 * units::us, Ray(vertex, vertex + Vector(0, 0, 0.1 * stepsize)),
                         -1.0 * units::eplus);
    }

    em("made tracks");

    Point field_origin(100 * units::mm, 0, 0);
    const double drift_speed = 1.0 * units::mm / units::us;
    const double nsigma = 3.0;
    IRandom::pointer fluctuate = nullptr;
    /// Time/Pitch Gaussian
    const double sigma_time = 0.0 * units::us;
    const double sigma_pitch = 0.3 * units::mm;

    /// Time bins
    const double t_sample = 0.5 * units::us;
    const double t_min = event_time - 1 * units::s;
    const double t_max = event_time + 1 * units::s;
    const int nt = (t_max - t_min) / t_sample;
    const Binning tbins(nt, t_min, t_max);

    /// Pitch bins
    const double p_sample = 0.3 * units::mm;
    const double p_min = -10 * p_sample;
    const double p_max = 10 * p_sample;
    const int np = (p_max - p_min) / p_sample;
    const Binning pbins(np, p_min, p_max);

    /// output file
    // TFile* rootfile = TFile::Open(Form("%s.root", out_basename.c_str()), "recreate");
    TCanvas *canvas = new TCanvas("c", "canvas", 1000, 1000);
    gStyle->SetOptStat(0);
    Double_t x[6];
    Double_t y[6];
    Double_t xc[6];  // cross check
    Double_t yc[6];

    // point charge case, if at right boundary of the bin, in next impact pitch, not shown in the following. Other cases
    // will overwrite.
    x[5] = 0.3;
    y[5] = 0;
    xc[5] = 0.3;
    yc[5] = 0;

    // charge distribution
    TF1 *charge[6];

    int pcount = 0;
    int wcount = 0;

    auto depos = tracks.depos();
    for (auto depo : depos) {
        auto drifted = std::make_shared<Gen::TransportedDepo>(depo, field_origin.x(), drift_speed);
        double center_time = drifted->time();
        double center_pitch = drifted->pos().z();
        std::cerr << "depo:"
                  << " q=" << drifted->charge() / units::eplus << "ele"
                  << " time-T0=" << (drifted->time() - t0) / units::us << "us"
                  << " pt=" << drifted->pos() / units::mm << " mm\n";

        Gen::GausDesc tdesc(center_time, sigma_time);
        Gen::GausDesc pdesc(center_pitch, sigma_pitch);
        if (!sigma_pitch) {
            charge[5 - pcount] = new TF1(Form("charge%d", 5 - pcount), "1", center_pitch - 0.01, center_pitch + 0.01);
            cout << "hahha" << endl;
        }
        else {
            charge[5 - pcount] = new TF1(Form("charge%d", 5 - pcount), "[0]*TMath::Gaus(x, [1], [2], 0)", -0.3, 0.9);
            charge[5 - pcount]->SetParameters(1, center_pitch, sigma_pitch);
            charge[5 - pcount]->SetNpx(1000);
        }

        Gen::GaussianDiffusion gd(drifted, tdesc, pdesc);
        gd.set_sampling(tbins, pbins, nsigma, fluctuate);

        int offsetbin = gd.poffset_bin();
        cout << "Offset bin: " << offsetbin << std::endl;
        auto weight = gd.weights();
        for (unsigned int i = 0; i < weight.size(); i++) {
            if (10 - offsetbin - i == 0) {
                cout << "weight " << i << " : " << weight[i] << endl;
                double check = WeightGaus(pbins.edge(10), pbins.edge(10) + p_sample, center_pitch, sigma_pitch);
                cout << "weight " << check << endl;
                y[5 - pcount] = weight[i];
                x[5 - pcount] = center_pitch;
                yc[5 - pcount] = check;
                xc[5 - pcount] = center_pitch;
                wcount++;
            }
        }
        pcount++;
    }
    cout << "Points: " << pcount << endl;
    cout << "Weights: " << wcount << endl;
    for (int i = 0; i < pcount; i++) {
        cout << x[i] << ", " << y[i] << endl;
    }

    /// Plot
    TGraph *graph = new TGraph(6, x, y);
    graph->SetTitle("Gaussian");
    TGraph *graphc = new TGraph(6, xc, yc);
    graph->Draw("A*");
    graph->GetXaxis()->SetLimits(-0.3, 0.9);
    graph->GetYaxis()->SetRangeUser(0, 1.1);
    graphc->Fit("pol1");
    graphc->Draw("* same");
    graphc->SetMarkerColor(kRed);
    graphc->SetMarkerStyle(24);
    graphc->SetMarkerSize(2);
    TBox bb(0, 0, 0.3, 1.1);
    bb.SetLineColor(kRed);
    bb.SetFillColorAlpha(7, 0.4);
    bb.Draw("same");

    Int_t color[6] = {2, 4, 6, 8, 9, 1};
    for (int i = 0; i < pcount; i++) {
        charge[i]->Draw("same");
        charge[i]->SetLineColor(color[i]);
        charge[i]->SetLineStyle(kDashed);
    }

    canvas->Print(Form("%s-Gaus7.pdf", out_basename.c_str()), "pdf");

    return 0;
}
