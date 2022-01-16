
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Units.h"

#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Binning.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TLine.h"
#include "TStyle.h"
#include "TH2F.h"

#include <vector>
#include <iostream>
#include <map>

using namespace WireCell;
using namespace std;

std::vector<TH1F*> plot_wave(TCanvas& canvas, int padnum, std::string name, std::pair<double, double> bounds,
                             const Binning& bins, const Waveform::realseq_t& tdomain,
                             const Waveform::compseq_t& fdomain)
{
    const int nbins = bins.nbins();
    const double vmax = (1.0 / (bins.binsize() / units::second)) * 1e-6;
    const double vbin = vmax / nbins;

    TH1F* ht =
        new TH1F("ht", Form("%s - waveform", name.c_str()), nbins, bins.min() / units::ms, bins.max() / units::ms);
    ht->SetXTitle(Form("time (ms, %d bins)", nbins));
    ht->SetDirectory(0);
    ht->GetXaxis()->SetRangeUser(bounds.first, bounds.second);

    TH1F* hm = new TH1F("hm", Form("%s - magnitude", name.c_str()), nbins, 0, vmax);
    hm->SetXTitle(Form("frequency (MHz, %d bins)", nbins));
    hm->SetDirectory(0);

    TH1F* hp = new TH1F("hp", Form("%s - phase", name.c_str()), nbins, 0, vmax);
    hp->SetXTitle(Form("frequency (MHz, %d bins)", nbins));
    hp->SetDirectory(0);

    for (int ind = 0; ind < bins.nbins(); ++ind) {
        const double tms = bins.center(ind);
        const double fMHz = (ind + 0.5) * vbin;
        ht->Fill(tms / units::ms, tdomain[ind]);
        hm->Fill(fMHz, std::abs(fdomain[ind]));
        hp->Fill(fMHz, std::arg(fdomain[ind]));
    }

    TVirtualPad* gpad = 0;

    gpad = canvas.cd(padnum++);
    ht->Draw("HIST");

    gpad = canvas.cd(padnum++);
    gpad->SetLogy(1);
    hm->Draw("HIST");

    gpad = canvas.cd(padnum++);
    hp->Draw("HIST");

    return std::vector<TH1F*>{ht, hm, hp};
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "This test requires an Wire Cell Field Response input file." << std::endl;
        return 0;
    }

    const char* uvw = "UVW";

    // time binning
    const int nticks = 10000;
    const double tick = 0.5 * units::us;
    Binning bins(nticks, 0, nticks * tick);

    // Get one field response (the one nearest the WOI)
    auto fr = Response::Schema::load(argv[1]);
    cerr << "Drift speed is: " << fr.speed / (units::mm / units::us) << " mm/us\n";

    TCanvas canvas("h", "h", 900, 1200);
    canvas.Print(Form("%s.pdf[", argv[0]), "pdf");

    const int nplanes = 3;
    for (int iplane = 0; iplane < nplanes; ++iplane) {
        auto plane_response = fr.planes[iplane];

        const int npaths = plane_response.paths.size();

        for (int ipath = 0; ipath < npaths; ++ipath) {
            auto& path_response = plane_response.paths[ipath];

            WireCell::Waveform::realseq_t raw_response = path_response.current;

            const int rawresp_size = raw_response.size();
            Assert(rawresp_size);
            const double rawresp_min = fr.tstart;
            const double rawresp_tick = fr.period;
            const double rawresp_max = rawresp_min + rawresp_size * rawresp_tick;
            Binning rawresp_bins(rawresp_size, rawresp_min, rawresp_max);

            // match response sampling to digi and zero-pad
            WireCell::Waveform::realseq_t response(nticks, 0.0);
            for (int ind = 0; ind < rawresp_size; ++ind) {
                const double time = rawresp_bins.center(ind);
                const int bin = bins.bin(time);
                response[bin] += raw_response[ind];
            }

            // Make up a Gaussian charge distribution
            const double charge_const = 1000.0;
            const double charge_time = 1 * units::ms;
            const double charge_sigma = 3 * units::us;

            WireCell::Waveform::realseq_t electrons(nticks, 0.0);
            for (int ind = 0; ind < nticks; ++ind) {
                const double t = bins.center(ind);
                const double rel = (t - charge_time) / charge_sigma;
                const double val = charge_const * exp(-0.5 * rel * rel);
                electrons[ind] = val;
            }

            // frequency space
            Waveform::compseq_t charge_spectrum = Waveform::dft(electrons);
            Waveform::compseq_t raw_response_spectrum = Waveform::dft(raw_response);
            Waveform::compseq_t response_spectrum = Waveform::dft(response);

            // convolve
            Waveform::compseq_t conv_spectrum(nticks, Waveform::complex_t(0.0, 0.0));
            for (int ind = 0; ind < nticks; ++ind) {
                conv_spectrum[ind] = response_spectrum[ind] * charge_spectrum[ind];
            }
            Waveform::realseq_t conv = Waveform::idft(conv_spectrum);
            for (int ind = 0; ind < nticks; ++ind) {
                conv[ind] /= nticks;
            }

            canvas.Clear();
            canvas.Divide(3, 4);

            std::string extra = Form(" %c-%.1f", uvw[iplane], path_response.pitchpos);

            plot_wave(canvas, 1, "Response" + extra, std::make_pair(0, .1), rawresp_bins, raw_response,
                      raw_response_spectrum);
            plot_wave(canvas, 4, "Response" + extra, std::make_pair(0, .1), bins, response, response_spectrum);
            plot_wave(canvas, 7, "Electrons" + extra, std::make_pair(.9, 1.1), bins, electrons, charge_spectrum);
            plot_wave(canvas, 10, "Signal" + extra, std::make_pair(.9, 1.1), bins, conv, conv_spectrum);

            canvas.Print(Form("%s.pdf", argv[0]), "pdf");

        }  // paths
    }      // planes

    canvas.Print(Form("%s.pdf]", argv[0]), "pdf");
    return 0;
}
