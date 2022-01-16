// Test RCResponse
#include "MultiPdf.h"  // local helper shared by a few tests

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellGen/RCResponse.h"

#include "TGraph.h"
#include "TH1F.h"

using namespace WireCell;

int main(int argc, char* argv[])
{
    Test::MultiPdf mpdf(argv[0]);

    const double tick = 0.5*units::us;
    const int nticks = 10000;
    Gen::RCResponse rcr;

    auto cfg = rcr.default_configuration();
    cfg["tick"] = tick;
    cfg["nticks"] = nticks;
    const double F_hz = 1.0/(tick/units::s);
    const double f0_hz = F_hz/nticks;

    rcr.configure(cfg);
    const auto& wavep1 = rcr.waveform_samples();
    // skip first which holds delta
    Waveform::realseq_t wave(wavep1.begin()+1, wavep1.end());
    auto spec = Waveform::dft(wave);
    auto mag = Waveform::magnitude(spec);

    TGraph* g = new TGraph(wave.size());
    TGraph* a = new TGraph(mag.size());

    for (size_t ind=0; ind<mag.size(); ++ind) {
        const double t_us = (ind*tick)/units::us;
        g->SetPoint(ind, t_us, wave[ind]);
        if (ind > mag.size()/2) continue;
        const double f_mhz = ind*f0_hz/1e6;
        a->SetPoint(ind, f_mhz, mag[ind]);
    }

    mpdf.canvas.Divide(1, 2);

    auto pad = mpdf.canvas.cd(1);
    pad->SetGridx();
    pad->SetGridy();

    g->Draw("AL");
    auto frame = g->GetHistogram();
    frame->SetTitle("RC Responses (DC-suppressed)");
    frame->SetXTitle("time [us]");
    frame->SetYTitle("arb");

    pad = mpdf.canvas.cd(2);
    pad->SetLogy();
    pad->SetGridx();
    pad->SetGridy();
    a->Draw("AL");
    frame = a->GetHistogram();
    frame->SetTitle("RC Amplitude (DC-suppressed)");
    frame->SetXTitle("freq [MHz]");
    frame->SetYTitle("amp");

    mpdf();

    return 0;
}
