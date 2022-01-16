#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Response.h"

#include "TH1F.h"
#include "TLine.h"
#include "TText.h"
#include "TGraph.h"

#include "MultiPdf.h"

#include <iostream>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace WireCell;
using namespace WireCell::Test;

// The preferred display units for gain.
const double GUnit = units::mV / units::fC;

void draw_time_freq(MultiPdf& pdf, Waveform::realseq_t& res, const std::string& title, const Binning& tbins)
{
    Waveform::compseq_t spec = Waveform::dft(res);
    Waveform::realseq_t res2 = Waveform::idft(spec);

    TH1F h_wave("response", title.c_str(), tbins.nbins(), tbins.min() / units::us, tbins.max() / units::us);
    TH1F h_wave2("response2", title.c_str(), tbins.nbins(), tbins.min() / units::us, tbins.max() / units::us);
    cout << "tbins.min()" << tbins.min() << endl;
    cout << "tbins.max()" << tbins.max() << endl;
    h_wave2.SetLineColor(2);

    h_wave.SetXTitle("Time (microsecond)");
    h_wave.SetYTitle("Gain (mV/fC)");
    h_wave.GetXaxis()->SetRangeUser(0, 10.0);

    const int nticks = tbins.nbins();
    for (int ind = 0; ind < nticks; ++ind) {
        h_wave.Fill(tbins.center(ind) / units::us, res[ind] / GUnit);
        h_wave2.Fill(tbins.center(ind) / units::us, res2[ind] / GUnit);
    }

    cerr << nticks << " " << spec.size() << endl;

    const double tick = tbins.binsize();
    const Binning fbins(nticks, 0, 1 / tick);

    TH1F h_mag("mag", "Magnitude of Fourier transform of response", fbins.nbins(), fbins.min() / units::megahertz,
               fbins.max() / units::megahertz);
    h_mag.SetYTitle("Amplitude [mV/fC]");
    h_mag.SetXTitle("MHz");

    TH1F h_phi("phi", "Phase of Fourier transform of response", fbins.nbins(), fbins.min() / units::megahertz,
               fbins.max() / units::megahertz);
    h_phi.SetYTitle("Phase [radian]");
    h_phi.SetXTitle("MHz");

    for (int ind = 0; ind < nticks; ++ind) {
        auto c = spec[ind];
        const double freq = fbins.center(ind);
        h_mag.Fill(freq / units::megahertz, std::abs(c) / GUnit);
        h_phi.Fill(freq / units::megahertz, std::arg(c));
    }
    // h_mag.GetXaxis()->SetRangeUser(0,2.0);
    // h_phi.GetXaxis()->SetRangeUser(0,2.0);

    pdf.canvas.Clear();
    pdf.canvas.Divide(2, 1);

    auto pad = pdf.canvas.cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_wave.Draw("hist");
    h_wave2.Draw("hist,same");

    auto spad = pdf.canvas.cd(2);
    spad->Divide(1, 2);
    pad = spad->cd(1);
    pad->SetGridx();
    pad->SetGridy();
    h_mag.Draw("hist");

    TLine ndb;
    TText ndbtxt;
    double maxmag = h_mag.GetMaximum();
    double db3 = maxmag / 1.414;
    double db6 = maxmag / 1.995;
    double db10 = maxmag / 3.162;
    double db20 = maxmag / 10;
    ndb.DrawLine(0, db3, 1.0, db3);
    ndb.DrawLine(0, db6, 1.0, db6);
    ndb.DrawLine(0, db10, 1.0, db10);
    ndb.DrawLine(0, db20, 1.0, db20);
    ndbtxt.DrawText(0.5, db3, "-3dB");
    ndbtxt.DrawText(0.5, db6, "-6dB");
    ndbtxt.DrawText(0.5, db10, "-10dB");
    ndbtxt.DrawText(0.5, db20, "-20dB");

    pad = spad->cd(2);
    pad->SetGridx();
    pad->SetGridy();
    h_phi.Draw("hist");
    pdf();
}

int main(int argc, char* argv[])
{
    const std::vector<double> gains = {7.8 * GUnit, 14.0 * GUnit};
    const std::vector<double> shapings = {1.0 * units::us, 2.0 * units::us};

    const double maxtime = 100.0 * units::us;
    const double tick = 0.5 * units::us;
    const Binning tbins(maxtime / tick, 0, maxtime);
    const int nticks = tbins.nbins();
    cerr << "Using nticks=" << nticks << endl;

    MultiPdf pdf(argv[0]);

    for (size_t ind = 0; ind < gains.size(); ++ind) {
        Response::ColdElec ce(gains[ind], shapings[ind]);
        Waveform::realseq_t res = ce.generate(tbins);

        const double tshape_us = shapings[ind] / units::us;
        auto tit = Form("Cold Electronics Response at %.0fus peaking", tshape_us);
        draw_time_freq(pdf, res, tit, tbins);
    }

    // Look at RC filter

    const double tconst = 1000.0 * units::ms;

    {
        Response::SimpleRC rc(tconst);
        Waveform::realseq_t res = rc.generate(tbins);

        auto tit = "RC Response at 1ms time constant";
        draw_time_freq(pdf, res, tit, tbins);
    }
    {
        Binning shifted(tbins.nbins(), tbins.min() + tick, tbins.max() + tick);

        Response::SimpleRC rc(tconst);
        Waveform::realseq_t res = rc.generate(shifted);

        auto tit = "RC Response at 1ms time constant (suppress delta)";
        draw_time_freq(pdf, res, tit, tbins);
    }

    // Look at SysResp (Gaussian smear)
    {
        Response::SysResp gaus;
        Waveform::realseq_t res = gaus.generate(tbins);
        auto tit = "Response Gaussian smear by default";
        draw_time_freq(pdf, res, tit, tbins);
    }
    {
        double mag = 1.0;
        double smear = 2.0 * units::us;
        double nsigma = 5;
        Binning ttt(nsigma * 2 * smear / tick, -nsigma * smear, nsigma * smear);
        // Binning ttt(nsigma*2*smear/tick, 0, 2*nsigma*smear);
        Response::SysResp gaus(tick, mag, smear);
        Waveform::realseq_t res = gaus.generate(ttt);
        auto tit = "Response Gaussian 2 us smear";
        draw_time_freq(pdf, res, tit, ttt);
    }

    // do timing tests
    {
        TGraph* timings[4] = {new TGraph, new TGraph, new TGraph, new TGraph};

        // Some popular choices with powers-of-two sprinkled in
        std::vector<int> nsampleslist{128,   256,  400,  480,  // protoDUNE U/V and W channels per plane
                                      512,
                                      800,  // protoDUNE, sum of U or V channels for both faces
                                      960,  // protoDUNE, sum of W channels (or wires) for both faces
                                      1024,
                                      1148,  // N wires in U/V plane for protodune
                                      2048,
                                      2400,  // number of channels in U or V in microboone
                                      2560,  // DUNE, total APA channels
                                      3456,  // number of channels in microboone's W
                                      4096,
                                      6000,  // one choice of nticks for protoDUNE
                                      8192,
                                      8256,                     // total microboone channels
                                      9592,  9594, 9595, 9600,  // various microboone readout lengths
                                      10000,                    // 5 ms at 2MHz readout
                                      10240, 16384};
        const int ntries = 1000;
        for (auto nsamps : nsampleslist) {
            Response::ColdElec ce(gains[1], shapings[1]);
            const Binning bins(nsamps, 0, maxtime);
            Waveform::realseq_t res = ce.generate(bins);
            Waveform::compseq_t spec;

            double fwd_time = 0.0;
            for (int itry = 0; itry < ntries; ++itry) {
                auto t1 = std::chrono::high_resolution_clock::now();
                spec = Waveform::dft(res);
                auto t2 = std::chrono::high_resolution_clock::now();
                fwd_time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
            }
            fwd_time /= ntries;

            double rev_time = 0.0;
            for (int itry = 0; itry < ntries; ++itry) {
                auto t1 = std::chrono::high_resolution_clock::now();
                res = Waveform::idft(spec);
                auto t2 = std::chrono::high_resolution_clock::now();
                rev_time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
            }
            rev_time /= ntries;

            cerr << "DFT nsamples=" << nsamps << "\n\tforward: " << fwd_time / 1000.0 << " us, "
                 << fwd_time / nsamps / 1000.0 << " us/sample"
                 << "\n\treverse: " << rev_time / 1000.0 << " us, " << rev_time / nsamps / 1000.0 << " us/sample"
                 << "\n\taverage: " << 0.5 * (fwd_time + rev_time) / 1000.0 << " us" << endl;

            cout << "timing " << nsamps << " " << fwd_time / 1000.0 << " " << rev_time / 1000.0 << "\n";

            timings[0]->SetPoint(timings[0]->GetN(), nsamps, fwd_time);
            timings[1]->SetPoint(timings[1]->GetN(), nsamps, fwd_time / nsamps);
            timings[2]->SetPoint(timings[2]->GetN(), nsamps, rev_time);
            timings[3]->SetPoint(timings[3]->GetN(), nsamps, rev_time / nsamps);
        }

        pdf.canvas.Clear();
        pdf.canvas.Divide(1, 2);

        auto text = new TText;
        {
            auto pad = pdf.canvas.cd(1);
            pad->SetGridx();
            pad->SetGridy();
            pad->SetLogx();
            auto graph = timings[0];
            auto frame = graph->GetHistogram();
            frame->SetTitle("Fwd/rev DFT timing (absolute)");
            frame->GetXaxis()->SetTitle("number of samples");
            frame->GetYaxis()->SetTitle("time (ns)");
            timings[0]->Draw("AL");
            timings[2]->Draw("L");
            for (int ind = 0; ind < graph->GetN(); ++ind) {
                auto x = graph->GetX()[ind];
                auto y = graph->GetY()[ind];
                text->DrawText(x, y, Form("%.0f", x));
            }
        }

        {
            auto pad = pdf.canvas.cd(2);
            pad->SetGridx();
            pad->SetGridy();
            pad->SetLogx();
            auto frame = timings[1]->GetHistogram();
            frame->SetTitle("Fwd/rev DFT timing (relative)");
            frame->GetXaxis()->SetTitle("number of samples");
            frame->GetYaxis()->SetTitle("time per sample (ns/samp)");
            timings[1]->Draw("AL");
            timings[3]->Draw("L");
        }
        pdf();
    }

    return 0;
}
