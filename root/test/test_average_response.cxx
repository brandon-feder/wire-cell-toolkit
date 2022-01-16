#include "MultiPdf.h"  // local helper shared by a few tests

#include "WireCellUtil/Response.h"
#include "WireCellUtil/ExecMon.h"

#include "TH2F.h"
#include "TFile.h"
#include "TGraph.h"
#include "TFrame.h"

#include <iostream>
#include <string>
#include <algorithm>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void plot_plane_2d(MultiPdf& mpdf, const Response::Schema::FieldResponse& fr, int planeind, bool isavg)
{
    const char* uvw = "UVW";

    const Response::Schema::PlaneResponse& pr = fr.planes[planeind];
    const Response::Schema::PathResponse& par0 = pr.paths[0];
    const int ntbins = par0.current.size();
    const double tstart = fr.tstart;
    const double period = fr.period;
    const double pitch = pr.pitch;

    const char* type = "Fine";
    if (isavg) {
        type = "Average";
    }

    int minregion = 999, maxregion = -999;
    std::vector<double> impacts;
    for (auto path : pr.paths) {
        const int region = round((path.pitchpos - 0.001) / pitch);
        const double impact = path.pitchpos - region * pitch;
        // cerr << "region=" << region << " impact=" << impact <<" pitchpos=" << path.pitchpos << endl;
        minregion = std::min(minregion, region);
        maxregion = std::max(maxregion, region);
        if (region == 0) {
            impacts.push_back(impact);
        }
    }
    sort(impacts.begin(), impacts.end());
    double dimpact = impacts[1] - impacts[0];
    double maximpact = impacts[impacts.size() - 1];
    if (isavg) {
        maximpact = dimpact = 0.0;
    }

    int npitchbins = pr.paths.size();
    double minpitch = -npitchbins / 2;
    double maxpitch = +npitchbins / 2;
    if (!isavg) {
        maxpitch = maxregion * pitch + maximpact;
        minpitch = minregion * pitch - maximpact;
        npitchbins = (maxpitch - minpitch) / dimpact;
    }
    /*
    cerr << isavg
         << " tstart=" << tstart
         << " period=" << period
         << " ntbins=" << ntbins
         << endl
         << " npaths=" << npitchbins
         << " dimpact=" << dimpact
         << " maxregion=" << maxregion
         << " minregion=" << minregion
         << " maximpact=" << maximpact
         << " maxpitch=" << maxpitch
         << " minpitch=" << minpitch
         << " npitchbins=" <<  npitchbins
         << " impacts:" << impacts[0] << " -> " << impacts[impacts.size()-1]
         << endl;
    */

    TH2F h("h", Form("%s %c-plane", type, uvw[pr.planeid]), ntbins, tstart, tstart + ntbins * period, npitchbins,
           minpitch, maxpitch);
    h.SetXTitle("time [us]");
    h.SetYTitle("wire region");
    h.SetStats(false);
    // h.GetXaxis()->SetRangeUser(0, 100);
    for (auto path : pr.paths) {
        const Waveform::realseq_t& response = path.current;
        for (int ind = 0; ind < ntbins; ++ind) {
            double value = response[ind];
            const double t = tstart + period * ind;
            if (isavg) {
                h.Fill(t, path.pitchpos / pitch, value);
            }
            else {
                h.Fill(t, path.pitchpos + 0.5 * dimpact, value);
            }
        }
    }

    h.Draw("colz");
    mpdf();
}

void plot_all_impact(MultiPdf& mpdf, const Response::Schema::FieldResponse& fr, bool isavg)
{
    // plot Nregion X Nimpact

    const Response::Schema::PlaneResponse& p0r = fr.planes[0];
    const Response::Schema::PathResponse& p0p0r = p0r.paths[0];
    const int ntbins = p0p0r.current.size();
    const double tstart = fr.tstart;
    const double period = fr.period;
    const double pitch = p0r.pitch;

    // figure out what impacts and regions there are
    std::vector<double> impacts;
    std::vector<int> regions;
    for (auto path : p0r.paths) {
        const int region = round((path.pitchpos - 0.001) / pitch);
        const double impact = path.pitchpos - region * pitch;
        if (region == 0) {
            impacts.push_back(impact);
        }
        if (std::abs(impact) < 0.001) {
            regions.push_back(region);
        }
    }
    sort(impacts.begin(), impacts.end());
    sort(regions.begin(), regions.end());

    int plane_colors[] = {2, 4, 1};

    int nimpacts = impacts.size();
    int nregions = regions.size();
    std::vector<TH1F*> hists[3];
    for (int iplane = 0; iplane < 3; ++iplane) {
        hists[iplane].resize(nimpacts * nregions);
        for (int imp = 0; imp < nimpacts; ++imp) {
            for (int reg = 0; reg < nregions; ++reg) {
                int wire = reg - nregions / 2;

                char sign = ' ';  // be a bit anal
                if (wire > 0) {
                    sign = '+';
                }
                if (wire < 0) {
                    sign = '-';
                }

                std::string title;
                if (isavg) {
                    title = Form("Avg Response wire:%c%d", sign, std::abs(wire));
                }
                else {
                    title = Form("Fine Response wire:%c%d (impact=%.1f)", sign, std::abs(wire), impacts[imp]);
                }

                TH1F* h = new TH1F(Form("h_%d_%d_%d", iplane, imp, reg), title.c_str(), ntbins, tstart,
                                   tstart + period * ntbins);
                h->SetLineColor(plane_colors[iplane]);
                hists[iplane][imp * nregions + reg] = h;
            }
        }
    }

    for (auto plane : fr.planes) {
        int iplane = plane.planeid;
        for (auto path : plane.paths) {
            const Waveform::realseq_t& response = path.current;
            const int region = round((path.pitchpos - 0.001) / pitch);
            const double impact = path.pitchpos - region * pitch;

            int imp = -1, reg = -1;  // I hate C++
            for (size_t ind = 0; ind < impacts.size(); ++ind) {
                if (std::abs(impact - impacts[ind]) < 0.001) {
                    imp = ind;
                    break;
                }
            }
            for (size_t ind = 0; ind < regions.size(); ++ind) {
                if (std::abs(region - regions[ind]) < 0.001) {
                    reg = ind;
                    break;
                }
            }
            // cerr << "plane="<<iplane<<" imp="<<imp<<" impact="<<impact<<" reg="<<reg<<" region="<<region<<endl;
            TH1F* hist = hists[iplane][imp * nregions + reg];
            for (int ind = 0; ind < ntbins; ++ind) {
                double value = response[ind];
                hist->SetBinContent(ind + 1, value);
            }
        }
    }

    for (int reg = 0; reg < nregions; ++reg) {
        mpdf.canvas.Divide(1, nimpacts);
        for (int imp = 0; imp < nimpacts; ++imp) {
            mpdf.canvas.cd(imp + 1);
            double minval = 999, maxval = -999;
            for (int iplane = 0; iplane < 3; ++iplane) {
                TH1F* hist = hists[iplane][imp * nregions + reg];
                minval = min(minval, hist->GetMinimum());
                maxval = max(maxval, hist->GetMaximum());
            }
            // cerr << "imp="<<imp<<" reg="<<reg<<endl;
            double extraval = 0.01 * (maxval - minval);
            for (int iplane = 0; iplane < 3; ++iplane) {
                TH1F* hist = hists[iplane][imp * nregions + reg];
                hist->SetMinimum(minval - extraval);
                hist->SetMaximum(maxval + extraval);
                if (iplane) {  // I hate root
                    hist->Draw("same");
                }
                else {
                    hist->Draw();
                }
            }
        }
        mpdf();
    }

    for (int imp = 0; imp < nimpacts; ++imp) {
        for (int reg = 0; reg < nregions; ++reg) {
            for (int iplane = 0; iplane < 3; ++iplane) {
                TH1F* hist = hists[iplane][imp * nregions + reg];
                delete hist;
                hists[iplane][imp * nregions + reg] = nullptr;
            }
        }
    }
}

const std::vector<double> all_gains{4.7, 7.8, 14.0, 25.0};
const std::vector<double> all_shapings{0.5 * units::us, 1.0 * units::us, 2.0 * units::us, 3.0 * units::us};

typedef std::pair<int, int> GainShape;
typedef std::map<GainShape, Waveform::realseq_t> CerfMap;

CerfMap make_cerf(double tmin = 0, double tmax = 100 * units::us, int ntbins = 1000);
CerfMap make_cerf(double tmin, double tmax, int ntbins)
{
    const Waveform::Domain dom(tmin, tmax);
    CerfMap ret;
    for (int igain = 0; igain < 4; ++igain) {
        for (int ishap = 0; ishap < 4; ++ishap) {
            auto ele = Response::ColdElec(all_gains[igain], all_shapings[ishap]);
            auto rf = ele.generate(dom, ntbins);
            ret[GainShape(igain, ishap)] = rf;
        }
    }
    return ret;
}

void plot_cerf(MultiPdf& mpdf, CerfMap& cerf, double tmin = 0, double tmax = 100 * units::us, int ntbins = 1000);
void plot_cerf(MultiPdf& mpdf, CerfMap& cerf, double tmin, double tmax, int ntbins)
{
    const double deltat = (tmax - tmin) / ntbins;

    auto frame = mpdf.canvas.DrawFrame(0, 0, 10, 25);
    mpdf.canvas.SetGridx();
    mpdf.canvas.SetGridy();
    frame->SetTitle("Electronics Responses");
    frame->SetXTitle("time [us]");
    frame->SetYTitle("gain [mV/fC]");

    int colors[] = {1, 2, 4, 6};

    std::vector<TGraph*> garbage_collection;
    for (int igain = 0; igain < 4; ++igain) {
        for (int ishap = 0; ishap < 4; ++ishap) {
            auto rf = cerf[GainShape(igain, ishap)];
            TGraph* g = new TGraph(ntbins);
            for (int ind = 0; ind < ntbins; ++ind) {
                g->SetPoint(ind, (tmin + ind * deltat) / units::us, rf[ind]);
            }
            g->Draw("L");
            g->SetLineColor(colors[ishap]);
            garbage_collection.push_back(g);
        }
    }
    mpdf();
    for (auto g : garbage_collection) {
        delete g;
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        cerr << "This test requires an Wire Cell Field Response input file." << endl;
        return 0;
    }

    WireCell::ExecMon em("test_persist_responses");
    auto fr = Response::Schema::load(argv[1]);
    em("loaded");

    auto fravg = Response::wire_region_average(fr);
    em("averaged");

    auto all_cerf = make_cerf();

    {
        MultiPdf mpdf(argv[0]);
        mpdf.canvas.SetRightMargin(.15);

        plot_cerf(mpdf, all_cerf);

        for (int ind = 0; ind < 3; ++ind) {
            em("plot_plane");
            plot_plane_2d(mpdf, fr, ind, false);
        }
        plot_all_impact(mpdf, fr, false);
        em("done with fine responses");

        for (int ind = 0; ind < 3; ++ind) {
            em("plot_plane avg");
            plot_plane_2d(mpdf, fravg, ind, true);
        }
        plot_all_impact(mpdf, fravg, true);
        em("done with avg responses");
    }

    cerr << em.summary() << endl;
    return 0;
}
