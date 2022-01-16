#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"

/// needed to pretend like we are doing WCT internals
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IFilterWaveform.h"
#include "WireCellIface/IConfigurable.h"

#include "WireCellUtil/Waveform.h"

#include "TFile.h"
#include "TGraph.h"

using namespace WireCell;

int main(int argc, char *argv[])
{
    const std::string ncr_tn = "LfFilter";
    const std::string ncr_tn1 = "HfFilter";
    {
        PluginManager &pm = PluginManager::instance();
        pm.add("WireCellSigProc");

        auto incrcfg = Factory::lookup<IConfigurable>(ncr_tn, "lf1");
        auto cfg = incrcfg->default_configuration();
        cfg["nbins"] = 9594;
        cfg["max_freq"] = 1 * units::megahertz;
        cfg["tau"] = 0.02 * units::megahertz;
        incrcfg->configure(cfg);
    }

    {
        auto incrcfg = Factory::lookup<IConfigurable>(ncr_tn1, "hf1");
        auto cfg = incrcfg->default_configuration();
        cfg["nbins"] = 9594;
        cfg["max_freq"] = 1 * units::megahertz;
        cfg["sigma"] = 4 * units::megahertz;
        cfg["power"] = 2;
        cfg["flag"] = true;
        incrcfg->configure(cfg);
    }

    auto ncr = Factory::find<IFilterWaveform>(ncr_tn, "lf1");
    auto ncr1 = Factory::find<IFilterWaveform>(ncr_tn1, "hf1");

    const int nfbins = 100;
    auto wfs = ncr->filter_waveform(nfbins);
    auto wfs1 = ncr1->filter_waveform(nfbins);

    TFile *file = new TFile(Form("%s.root", argv[0]), "RECREATE");
    TGraph *g1 = new TGraph();
    TGraph *g2 = new TGraph();
    for (size_t i = 0; i != wfs.size(); i++) {
        g1->SetPoint(i, i, wfs.at(i));
        g2->SetPoint(i, i, wfs1.at(i));
    }
    g1->Write("g1");
    g2->Write("g2");
    file->Write();
    file->Close();

    return 0;
}
