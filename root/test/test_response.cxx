#include "WireCellUtil/Waveform.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Response.h"

#include "TCanvas.h"
#include "TH1F.h"

#include <algorithm>

using namespace WireCell;

struct Plotter {
    TCanvas canvas;
    std::string filename;
    Plotter(const std::string& fname)
      : canvas("test_response", "Response functions", 500, 500)
      , filename(fname)
    {
        canvas.Print((filename + "[").c_str(), "pdf");
    }
    ~Plotter() { canvas.Print((filename + "]").c_str(), "pdf"); }
    void operator()() { canvas.Print(filename.c_str(), "pdf"); }
};

std::pair<TH1F*, TH1F*> plot_response(Plotter& plt, double gain_unit = units::mV / units::fC);
std::pair<TH1F*, TH1F*> plot_response(Plotter& plt, double gain_unit)
{
    const double gain1 = 7.8 * gain_unit;
    const double gain2 = 14.0 * gain_unit;
    const double shaping1 = 1.0 * units::us;
    const double shaping2 = 2.0 * units::us;
    const double tmax = 10.0 * units::us;
    const double tick = 0.05 * units::us;

    const Binning bins(tmax / tick, 0, tmax);

    Response::ColdElec ce1(gain1, shaping1);
    Response::ColdElec ce2(gain2, shaping2);

    // exercise the generator
    Waveform::realseq_t res1 = ce1.generate(bins);
    Waveform::realseq_t res2 = ce2.generate(bins);

    TH1F* resp1 = new TH1F("resp1", "Cold Electronics Response at 1us shaping", bins.nbins(), bins.min() / units::us,
                           bins.max() / units::us);
    TH1F* resp2 = new TH1F("resp2", "Cold Electronics Response at 2us shaping", bins.nbins(), bins.min() / units::us,
                           bins.max() / units::us);
    resp1->SetLineColor(2);
    resp2->SetLineColor(4);
    for (size_t ind = 0; ind < res1.size(); ++ind) {
        const double t_us = bins.center(ind) / units::us;
        resp1->Fill(t_us, res1[ind] / gain_unit);
        resp2->Fill(t_us, res2[ind] / gain_unit);
    }

    plt.canvas.Clear();
    auto pad = &plt.canvas;
    pad->SetGridx();
    pad->SetGridy();
    TH1F* frame = pad->DrawFrame(0, 0, 10, 15, "Cold Electronics Response Functions (1us,7.8mV/fC and 2us,14.0mV/fC)");
    frame->SetXTitle("Time (microsecond)");
    if (gain_unit == units::mV / units::fC) {
        frame->SetYTitle("Gain (mV/fC)");
    }
    else {
        frame->SetYTitle("Gain");
    }
    resp1->Draw("hist,same");
    resp2->Draw("hist,same");

    plt();

    return std::make_pair(resp1, resp2);
}

void plot_ratio(Plotter& plt, TH1F* one, TH1F* two, const std::string& title)
{
    TH1F* rat = (TH1F*) two->Clone("rat");
    rat->Divide(one);

    plt.canvas.Clear();
    auto pad = &plt.canvas;
    pad->SetGridx();
    pad->SetGridy();

    const double eps = 0.01;
    TH1F* frame = pad->DrawFrame(0, 1 - eps, 10, 1 + eps, title.c_str());
    frame->SetXTitle("Time (microsecond)");
    frame->SetYTitle("gain ratio");
    pad->SetGridx();
    pad->SetGridy();
    rat->SetLineColor(1);
    rat->Draw("hist,same");

    plt();
}

int main(int argc, char* argv[])
{
    {
        Plotter plt(Form("%s.pdf", argv[0]));
        auto units = plot_response(plt);
        auto anons = plot_response(plt, 1.0);
        plot_ratio(plt, units.first, anons.first, "Ratio for 1us, 7.8mV/fC");
        plot_ratio(plt, units.second, anons.second, "Ratio for 2us, 14.0V/fC");
    }
}
