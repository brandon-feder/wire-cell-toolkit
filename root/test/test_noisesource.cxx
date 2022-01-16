#include "WireCellGen/EmpiricalNoiseModel.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IChannelSpectrum.h"
#include "WireCellIface/IFrameSource.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"
#include "TH2F.h"

#include <cstdlib>
#include <string>

#include "anode_loader.h"  // do not use this

using namespace std;
using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string detector = "uboone";
    if (argc > 1) {
        detector = argv[1];
    }
    auto anode_tns = anode_loader(detector);

    string filenames[3] = {
        "microboone-noise-spectra-v2.json.bz2",
    };

    ExecMon em;

    const int nticks = 1000;
    const double tick = 0.5 * units::us;
    const double readout_time = nticks * tick;

    // In a real WCT application all this configuration is done by
    // wire-cell (or hosting application) and driven by user
    // configuration files.  Here we have to expose some tedium.
    {
        auto icfg = Factory::lookup<IConfigurable>("Random");
        icfg->configure(icfg->default_configuration());
    }
    {
        auto icfg = Factory::lookup<IConfigurable>("StaticChannelStatus");
        auto cfg = icfg->default_configuration();
        cfg["anode"] = anode_tns[0];
        icfg->configure(cfg);
    }
    {
        auto icfg = Factory::lookup<IConfigurable>("EmpiricalNoiseModel");
        auto cfg = icfg->default_configuration();
        cfg["spectra_file"] = filenames[0];
        cfg["period"] = tick;
        cfg["nsamples"] = nticks;
        cfg["anode"] = anode_tns[0];
        icfg->configure(cfg);
    }
    {
        auto icfg = Factory::lookup<IConfigurable>("NoiseSource");
        auto cfg = icfg->default_configuration();
        cfg["anode"] = anode_tns[0];
        cfg["model"] = "EmpiricalNoiseModel";
        cfg["rng"] = "Random";
        cfg["readout_time"] = readout_time;
        cfg["m_nsamples"] = nticks;
        icfg->configure(cfg);
    }

    em("configuration done");

    auto noisesrc = Factory::lookup<IFrameSource>("NoiseSource");

    IFrame::pointer frame;

    bool ok = (*noisesrc)(frame);
    em("got noise frame");
    Assert(ok);

    // WARNING: if you are reading this for ideas of how to use frames
    // and traces beware that, for brevity, this test assumes the
    // frame is a filled in rectangle of channels X ticks and with
    // indexed channel numbers.  In general this is not true!

    auto traces = frame->traces();
    const int ntraces = traces->size();

    std::cout << nticks << " " << traces->at(0)->charge().size() << std::endl;

    Assert(nticks == traces->at(0)->charge().size());

    string tfilename = Form("%s.root", argv[0]);
    cerr << tfilename << endl;
    TFile* rootfile = TFile::Open(tfilename.c_str(), "recreate");
    // TCanvas* canvas = new TCanvas("c","canvas",1000,1000);
    // gStyle->SetOptStat(0);
    TH2F* hist = new TH2F("noise", "Noise Frame", nticks, 0, nticks, ntraces, 0, ntraces);
    for (auto trace : *traces) {
        int chid = trace->channel();
        const auto& qvec = trace->charge();
        for (int ind = 0; ind < nticks; ++ind) {
            // convert to ADC ...
            hist->Fill(ind + 0.5, chid + 0.5, qvec[ind] / units::mV * 4096 / 2000.);
        }
    }
    em("filled histogram");
    hist->Write();
    rootfile->Close();
    em("closed ROOT file");

    cerr << em.summary() << endl;

    return 0;
}
