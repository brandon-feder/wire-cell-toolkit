#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"

/// needed to pretend like we are doing WCT internals
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IChannelResponse.h"
#include "WireCellIface/IConfigurable.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TAxis.h"

#include <vector>
#include <string>
#include <iostream>

using namespace WireCell;
using WireCell::units::fC;
using WireCell::units::mV;
using WireCell::units::us;

using namespace std;

int main(int argc, char* argv[])
{
    // user code should never do this
    {
        PluginManager& pm = PluginManager::instance();
        pm.add("WireCellSigProc");
    }

    Binning binning(100, 0, 10 * us);

    /// The typename of the component to use here.  Normally this is
    /// given in the configuration for whatever component wants to use
    /// the channel response.
    const std::string ncr_tn = "NominalChannelResponse";

    /// User code should never do this but in this test we will abuse
    /// the configuration mechanism to reuse the same component to
    /// draw many responses.
    auto incrcfg = Factory::lookup<IConfigurable>(ncr_tn);
    auto cfg = incrcfg->default_configuration();
    cfg["nbins"] = binning.nbins();
    cfg["tmin"] = binning.min();
    cfg["tmax"] = binning.max();
    incrcfg->configure(cfg);

    /// Finally, we pretend to be user code.
    auto ncr = Factory::find<IChannelResponse>(ncr_tn);

    const double GU = mV / fC;

    const std::vector<double> gains{7.8 * GU, 14 * GU};
    const std::vector<double> shapes{0.5 * us, 1.0 * us, 2.0 * us, 3.0 * us};

    std::vector<TGraph*> graphs;

    for (auto gain : gains) {
        cfg["gain"] = gain;
        for (auto shape : shapes) {
            cfg["shaping"] = shape;

            /// WARNING: user code should never call configure().  We
            /// are abusing the system here to keep this test short.
            incrcfg->configure(cfg);
            /// Users: do not do this.

            auto wave = ncr->channel_response(0);
            const int nbins = wave.size();
            cerr << nbins << " " << binning.nbins() << endl;
            Assert(nbins == binning.nbins());
            TGraph* g = new TGraph(binning.nbins());
            for (int ind = 0; ind < binning.nbins(); ++ind) {
                g->SetPoint(ind, binning.center(ind) / us, wave[ind] / GU);
            }
            graphs.push_back(g);
        }
    }

    const int colors[] = {1, 2, 4, 6};
    TCanvas c("c", "c", 500, 500);
    auto frame = c.DrawFrame(0.0, 0.0, 10.0, 15.0);
    frame->SetTitle("Nominal Channel Response various gains/shaping");
    frame->GetXaxis()->SetTitle("time [us]");
    frame->GetYaxis()->SetTitle("gain [mV/fC]");
    for (size_t ind = 0; ind < graphs.size(); ++ind) {
        TGraph* g = graphs[ind];
        g->SetLineColor(colors[ind % 4]);
        g->SetMarkerColor(colors[ind % 4]);
        g->Draw("C*");
    }
    c.Print(Form("%s.pdf", argv[0]), "pdf");

    return 0;
}
