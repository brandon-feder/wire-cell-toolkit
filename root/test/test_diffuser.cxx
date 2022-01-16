#include "WireCellGen/Diffuser.h"

#include "WireCellIface/IDiffusion.h"

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Units.h"

#include "TH2F.h"
#include "TStyle.h"
#include "TPolyMarker.h"
#include "MultiPdf.h"

#include <iostream>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void dump_smear(IDiffusion::pointer smear)
{
    cerr << "L: [ " << smear->lpos(0) << " , " << smear->lpos(smear->lsize()) << " ]" << endl;
    cerr << "L: [ " << smear->tpos(0) << " , " << smear->tpos(smear->tsize()) << " ]" << endl;

    for (int tind = 0; tind < smear->tsize(); ++tind) {
        for (int lind = 0; lind < smear->lsize(); ++lind) {
            cerr << "\t" << smear->get(lind, tind);
        }
        cerr << endl;
    }
}

void test_one()
{
    Ray pitch(Point(0, 0, 0), Point(0, 0, 10));

    // binsize_l, binsize_t
    Diffuser diff(pitch, 1000);
    // mean_l, mean_t, sigma_l, sigma_t
    dump_smear(diff.diffuse(20000, 20, 2000, 2));

    for (double mean = -100; mean <= 100; mean += 0.11) {
        Diffuser::bounds_type bb = diff.bounds(mean, 1.5, 1.0);
        // cerr << "mean=" << mean << " [" << bb.first << " --> " << bb.second << "]" << endl;
        Assert(bb.second - bb.first == 10.0);
    }
}

void test_plot_hist(MultiPdf& pdf)
{
    pdf.canvas.Clear();

    // const int nsigma = 3;
    const double drift_velocity = 1.6 * units::mm / units::microsecond;
    const double binsize_l = 0.5 * units::microsecond * drift_velocity;
    const double binsize_t = 5 * units::mm;

    const double sigma_l = 3.0 * units::microsecond * drift_velocity;
    const double sigma_t = 3 * units::mm;

    Ray pitch(Point(0, 0, 0), Point(0, 0, binsize_t));

    Diffuser diff(pitch, binsize_l);

    vector<IDiffusion::pointer> diffs;
    vector<pair<double, double> > pts;

    // make a diagnonal
    for (double step = 0; step < 200; step += 5) {
        double mean_l = (0.5 + step) * binsize_l;
        double mean_t = (0.5 + step) * binsize_t;
        diffs.push_back(diff.diffuse(mean_l, mean_t, sigma_l, sigma_t));
        pts.push_back(make_pair(mean_l, mean_t));
    }

    // and two isolated dots
    diffs.push_back(diff.diffuse(10 * binsize_l, 100 * binsize_t, 3 * sigma_l, 3 * sigma_t, 10.0));
    diffs.push_back(diff.diffuse(100 * binsize_l, 10 * binsize_t, 3 * sigma_l, 3 * sigma_t, 10.0));

    double min_l = 0, min_t = 0, max_l = 0, max_t = 0;
    for (auto d : diffs) {
        min_l = min(min_l, d->lpos(0));
        min_t = min(min_t, d->tpos(0));
        max_l = max(max_l, d->lpos(d->lsize()));
        max_t = max(max_t, d->tpos(d->tsize()));
    }

    TH2F* h = new TH2F("smear", "Smear", (max_l - min_l) / binsize_l, min_l, max_l, (max_t - min_t) / binsize_t, min_t,
                       max_t);
    for (auto smear : diffs) {
        for (int tind = 0; tind < smear->tsize(); ++tind) {
            for (int lind = 0; lind < smear->lsize(); ++lind) {
                h->Fill(smear->lpos(lind, 0.5), smear->tpos(tind, 0.5), smear->get(lind, tind));
            }
        }
    }

    h->SetXTitle("Longitudinal direction");
    h->SetYTitle("Transverse direction");
    h->Draw("colz");

    TPolyMarker* pm = new TPolyMarker;
    pm->SetMarkerColor(5);
    pm->SetMarkerStyle(8);
    int count = 0;
    for (auto xy : pts) {
        pm->SetPoint(count++, xy.first, xy.second);
    }
    pm->Draw();

    gStyle->SetOptStat(11111111);
    pdf();
}

int main(int argc, char* argv[])
{
    test_one();

    MultiPdf pdf(argv[0]);

    test_plot_hist(pdf);

    return 0;
}
