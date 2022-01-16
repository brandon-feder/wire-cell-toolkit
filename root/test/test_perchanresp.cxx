#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"

/// needed to pretend like we are doing WCT internals
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IChannelResponse.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IConfigurable.h"

#include "TCanvas.h"
#include "TStyle.h"
#include "TH2F.h"
#include "TAxis.h"
#include "TFile.h"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include "anode_loader.h"  // do not use this

using namespace WireCell;
using WireCell::units::fC;
using WireCell::units::mV;
using WireCell::units::us;

using namespace std;

int main(int argc, char* argv[])
{
    std::string detector = "uboone";
    if (argc > 1) {
        detector = argv[1];
    }
    auto anode_tns = anode_loader(detector);

    // Real component code would get this info from its
    // configuration.
    const std::string cr_tn = "PerChannelResponse";
    const std::string ap_tn = anode_tns[0];
    const std::string pcr_filename = "microboone-channel-responses-v1.json.bz2";

    {  // User code should never do this.
        auto icfg = Factory::lookup<IConfigurable>(cr_tn);
        auto cfg = icfg->default_configuration();
        cfg["filename"] = pcr_filename;
        icfg->configure(cfg);
    }

    /// Finally, we now pretend to be real component code.
    auto cr = Factory::find_tn<IChannelResponse>(cr_tn);
    auto ap = Factory::find_tn<IAnodePlane>(ap_tn);

    auto bins = cr->channel_response_binning();
    cerr << "PerChannelResponse with binning: " << bins.nbins() << " bins "
         << " with sample period " << bins.binsize() / units::us << " us and bounds:"
         << "[" << bins.min() / units::us << "," << bins.max() / units::us << "]us\n";

    std::vector<int> planechans[3];  // fixme: will break with DUNE
    for (auto ch : ap->channels()) {
        auto wpid = ap->resolve(ch);
        planechans[wpid.index()].push_back(ch);
    }

    auto outfile = TFile::Open(Form("%s.root", argv[0]), "RECREATE");

    gStyle->SetOptStat(0);
    TCanvas c("c", "c", 500, 500);
    c.Divide(3, 1);

    // Desired gain units for showing in the plot
    const double GU = units::mV / units::fC;

    for (int iplane = 0; iplane < 3; ++iplane) {
        auto& channels = planechans[iplane];
        std::sort(channels.begin(), channels.end());

        /// assume all responses in a plane are the same size.
        const int nsamps = bins.nbins();
        const double mintus = bins.min() / units::us;
        const double maxtus = bins.max() / units::us;
        const int nchans = channels.size();

        Assert(nsamps > 0);
        Assert(nchans > 0);

        TH2F* hist = new TH2F(Form("hist%d", iplane), Form("Per Channel Response Plane %d [mV/fC]", iplane), nsamps,
                              mintus, maxtus, nchans, 0, nchans);
        hist->GetXaxis()->SetTitle("sample time (us)");
        hist->GetYaxis()->SetTitle("channel indices");

        for (int ich = 0; ich < nchans; ++ich) {
            const auto& resp = cr->channel_response(channels[ich]);
            for (int isamp = 0; isamp < nsamps; ++isamp) {
                const double Tus = bins.center(isamp) / units::us;
                hist->Fill(Tus, ich + 0.5, resp[isamp] / GU);
            }
        }

        c.cd(iplane + 1);
        hist->Draw("colz");
        hist->Write();
    }

    cerr << "Now ROOT makes the PDF" << endl;
    c.Print(Form("%s.pdf", argv[0]), "pdf");

    outfile->Close();

    return 0;
}
