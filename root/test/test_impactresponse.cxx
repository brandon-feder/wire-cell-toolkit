#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Logging.h"

#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Response.h"

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFieldResponse.h"
#include "WireCellIface/IPlaneImpactResponse.h"

#include "MultiPdf.h"  // local helper shared by a few tests
#include "TH2F.h"
#include "TLine.h"
#include "TStyle.h"
#include "TFile.h"

#include <vector>
#include <algorithm>

#include "Utils.h"

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

using spdlog::debug;
using spdlog::error;

void plot_time(MultiPdf& mpdf, IPlaneImpactResponse::pointer pir, int iplane, Binning tbins, const std::string& name,
               const std::string& title)
{
    // only show bins where we think the response is
    const double tmin = tbins.min();
    const double tspan = 100 * units::us;
    const int ntbins = tbins.bin(tmin + tspan);
    const double tmax = tbins.edge(ntbins);

    const int nwires = pir->nwires();

    const char* uvw = "UVW";

    const double half_pitch = 0.5 * pir->pitch_range();
    const double impact_dist = pir->impact();

    //const double pmin = -36 * units::mm, pmax = 36 * units::mm;
    const double pmin = -half_pitch;
    const double pmax =  half_pitch;
    const int npbins = (pmax - pmin) / impact_dist;

    // dr:
    std::string zunit = "negative microvolt";
    double zunitval = -units::microvolt;
    vector<double> zextent{1.0, 1.0, 2.0};
    if (name == "fr") {
        zunit = "induced electrons";
        zunitval = -units::eplus;
        zextent = vector<double>{0.3, 0.15, 0.6};
    }
    debug("Plane {}: nwires={}, half_pitch={:f} mm, impact_dist={:f} mm, zunit={} tbinsize={:f} us",
          iplane, nwires, half_pitch/units::mm, impact_dist/units::mm,
          zunit, tbins.binsize() / units::us);

    // they all suck.  black body sucks the least.
    set_palette(kBlackBody);
    // set_palette(kLightTemperature);
    // set_palette(kRedBlue);
    // set_palette(kTemperatureMap);
    // set_palette(kThermometer);
    // set_palette(kVisibleSpectrum);
    // set_palette();
    gStyle->SetOptStat(0);
    TH2F* hist =
        new TH2F(Form("h%s_%c", name.c_str(), uvw[iplane]),
                 Form("%s, 1e-/impact %c-plane", title.c_str(), uvw[iplane]),
                 ntbins, tmin / units::us, tmax / units::us,
                 npbins, pmin / units::mm, pmax / units::mm);
    hist->SetXTitle("time [us]");
    hist->SetYTitle("pitch [mm]");
    hist->SetZTitle(zunit.c_str());

    hist->GetZaxis()->SetRangeUser(-zextent[iplane], +zextent[iplane]);

    TH1F* htot = new TH1F(Form("htot%s_%c", name.c_str(), uvw[iplane]),
                          Form("%s total, 1e-/impact %c-plane",
                               title.c_str(), uvw[iplane]), npbins, pmin / units::mm,
                          pmax / units::mm);
    htot->SetXTitle("pitch (mm)");
    htot->SetYTitle(Form("impact total [%s]", zunit.c_str()));

    for (double pitch = -half_pitch; pitch <= half_pitch; pitch += impact_dist) {
        double eps = 0.0;
        if (pitch == -half_pitch) {
            eps = 1e-9;
        }
        IImpactResponse::pointer ir;
        //try {
        ir = pir->closest(pitch+eps);
        // }
        // catch (const ValueError& e) {
        //     error("No closest for pitch {} mm", pitch/units::mm);
        //     continue;
        // }
        auto spec = ir->spectrum();
        auto wave = Waveform::idft(spec);
        pitch += 0.001 * impact_dist;
        for (int ind = 0; ind < ntbins; ++ind) {
            const double time = tbins.center(ind);
            hist->Fill(time / units::us, pitch / units::mm, wave[ind] / zunitval);
            htot->Fill(pitch / units::mm, wave[ind] / zunitval);
        }
    }
    hist->Write();
    hist->Draw("colz");

    mpdf.canvas.SetRightMargin(0.15);
    mpdf.canvas.SetLeftMargin(0.15);

    TLine wline, hline;
    wline.SetLineColorAlpha(1, 0.5);
    wline.SetLineStyle(1);
    hline.SetLineColorAlpha(2, 0.5);
    hline.SetLineStyle(2);
    for (int iwire = 0; iwire < nwires / 2; ++iwire) {
        double wpitch = iwire * pir->pitch();
        if (wpitch < pmax) {
            wline.DrawLine(tmin / units::us, wpitch, tmax / units::us, wpitch);
            wline.DrawLine(tmin / units::us, -wpitch, tmax / units::us, -wpitch);
        }
        wpitch += 0.5 * pir->pitch();
        if (wpitch < pmax) {
            hline.DrawLine(tmin / units::us, wpitch, tmax / units::us, wpitch);
            hline.DrawLine(tmin / units::us, -wpitch, tmax / units::us, -wpitch);
        }
    }

    mpdf();
    htot->Draw("hist");
    mpdf();
}

int main(int argc, const char* argv[])
{
    Log::add_stdout(true, "debug");
    Log::set_level("debug");

    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    pm.add("WireCellSigProc");

    const int nticks = 9595;
    const double tick = 0.5 * units::us;
    const double gain = 14.0 * units::mV / units::fC;
    const double shaping = 2.0 * units::us;

    const double t0 = 0.0 * units::s;
    const double readout_time = nticks * tick;

    Binning tbins(nticks, t0, t0 + readout_time);

    string out_basename = argv[0];
    string response_file = "ub-10-half.json.bz2";
    if (argc > 1) {
        response_file = argv[1];
    };
    if (argc > 2) {
        out_basename = argv[2];
    }
    debug("Using response file: {} plotting to {}",
          response_file, out_basename);

    const std::string er_tn = "ColdElecResponse", rc_tn = "RCResponse";

    {  // configure elecresponse
        auto icfg = Factory::lookup_tn<IConfigurable>(er_tn);
        auto cfg = icfg->default_configuration();
        cfg["gain"] = gain;
        cfg["shaping"] = shaping;
        cfg["nticks"] = nticks;
        debug("Setting: {} ticks", cfg["nticks"].asInt());
        cfg["tick"] = tick;
        cfg["start"] = t0;
        icfg->configure(cfg);
    }
    {  // configure rc response
        auto icfg = Factory::lookup_tn<IConfigurable>(rc_tn);
        auto cfg = icfg->default_configuration();
        cfg["nticks"] = nticks;
        cfg["tick"] = tick;
        cfg["start"] = t0;
        icfg->configure(cfg);
    }
    {
        auto icfg = Factory::lookup<IConfigurable>("FieldResponse");
        auto cfg = icfg->default_configuration();
        cfg["filename"] = response_file;
        icfg->configure(cfg);
    }

    std::vector<std::string> pir_tns{"PlaneImpactResponse:frU",
        "PlaneImpactResponse:frV", "PlaneImpactResponse:frW"};
    {  // configure pirs, just FR
        for (int iplane = 0; iplane < 3; ++iplane) {
            auto icfg = Factory::lookup_tn<IConfigurable>(pir_tns[iplane]);
            auto cfg = icfg->default_configuration();
            cfg["plane"] = iplane;
            cfg["nticks"] = nticks;
            cfg["tick"] = tick;
            icfg->configure(cfg);
        }
    }
    std::vector<std::string> pir_ele_tns{"PlaneImpactResponse:frerU",
        "PlaneImpactResponse:frerV", "PlaneImpactResponse:frerW"};
    {  // configure pirs, FR + ER + RCRC
        for (int iplane = 0; iplane < 3; ++iplane) {
            auto icfg = Factory::lookup_tn<IConfigurable>(pir_ele_tns[iplane]);
            auto cfg = icfg->default_configuration();
            cfg["plane"] = iplane;
            cfg["nticks"] = nticks;
            cfg["tick"] = tick;
            cfg["other_responses"][0] = er_tn;
            cfg["other_responses"][1] = rc_tn;  // double it so
            cfg["other_responses"][2] = rc_tn;  // we get RC^2
            icfg->configure(cfg);
        }
    }

    // auto ifr = Factory::find_tn<IFieldResponse>("FieldResponse");
    // const auto& fr = ifr->field_response();

    TFile* rootfile = TFile::Open(Form("%s.root", out_basename.c_str()), "recreate");

    MultiPdf mpdf(out_basename.c_str());
    for (int iplane = 0; iplane < 3; ++iplane) {
        auto pir = Factory::find_tn<IPlaneImpactResponse>(pir_tns[iplane]);
        plot_time(mpdf, pir, iplane, tbins, "fr", "Field Response");

        auto pir_ele = Factory::find_tn<IPlaneImpactResponse>(pir_ele_tns[iplane]);
        plot_time(mpdf, pir_ele, iplane, tbins, "dr", "Detector Response");
    }

    mpdf.close();

    debug("Closing ROOT file: {}", rootfile->GetName());
    rootfile->Close();

    return 0;
}
