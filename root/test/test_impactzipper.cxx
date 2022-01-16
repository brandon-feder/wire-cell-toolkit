#include "WireCellGen/ImpactZipper.h"
#include "WireCellGen/TrackDepos.h"
#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellGen/TransportedDepo.h"
#include "WireCellGen/PlaneImpactResponse.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Response.h"

#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFieldResponse.h"
#include "WireCellIface/IPlaneImpactResponse.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TLine.h"
#include "TStyle.h"
#include "TH2F.h"

#include <iostream>
#include <string>

using namespace WireCell;
using namespace std;

int main(const int argc, char* argv[])
{
    string track_types = "point";
    if (argc > 1) {
        track_types = argv[1];
    }
    cerr << "Using tracks type: \"" << track_types << "\"\n";

    string response_file = "ub-10-half.json.bz2";
    if (argc > 2) {
        response_file = argv[2];
        cerr << "Using Wire Cell field response file:\n" << response_file << endl;
    }
    else {
        cerr << "No Wire Cell field response input file given, will try to use:\n" << response_file << endl;
    }

    string out_basename = argv[0];
    if (argc > 3) {
        out_basename = argv[3];
    }

    // here we do hard-wired configuration.  User code should NEVER do
    // this.

    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    pm.add("WireCellSigProc");
    {
        auto rngcfg = Factory::lookup<IConfigurable>("Random");
        auto cfg = rngcfg->default_configuration();
        rngcfg->configure(cfg);
    }

    const int nticks = 9595;
    const double tick = 0.5 * units::us;
    const double gain = 14.0 * units::mV / units::fC;
    const double shaping = 2.0 * units::us;

    const double t0 = 0.0 * units::s;
    const double readout_time = nticks * tick;
    const double drift_speed = 1.0 * units::mm / units::us;  // close, but not real

    const std::string er_tn = "ColdElecResponse", rc_tn = "RCResponse";

    {  // configure elecresponse
        auto icfg = Factory::lookup_tn<IConfigurable>(er_tn);
        auto cfg = icfg->default_configuration();
        cfg["gain"] = gain;
        cfg["shaping"] = shaping;
        cfg["nticks"] = nticks;
        cerr << "Setting: " << cfg["nticks"].asInt() << " ticks\n";
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

    std::vector<std::string> pir_tns{"PlaneImpactResponse:U", "PlaneImpactResponse:V", "PlaneImpactResponse:W"};
    {  // configure pirs
        for (int iplane = 0; iplane < 3; ++iplane) {
            auto icfg = Factory::lookup_tn<IConfigurable>(pir_tns[iplane]);
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

    WireCell::ExecMon em(out_basename);
    auto ifr = Factory::find_tn<IFieldResponse>("FieldResponse");
    auto fr = ifr->field_response();

    em("loaded response");

    const char* uvw = "UVW";

    // 1D garfield wires are all parallel
    const double angle = 60 * units::degree;
    const Vector upitch(0, -sin(angle), cos(angle));
    const Vector uwire(0, cos(angle), sin(angle));
    const Vector vpitch(0, sin(angle), cos(angle));
    const Vector vwire(0, cos(angle), -sin(angle));
    const Vector wpitch(0, 0, 1);
    const Vector wwire(0, 1, 0);

    // FIXME: need to apply electronics response!

    // Origin where drift and diffusion meets field response.
    Point field_origin(fr.origin, 0, 0);
    cerr << "Field response origin: " << field_origin / units::mm << "mm\n";

    // Describe the W collection plane
    const int nwires = 2001;
    const double wire_pitch = 3 * units::mm;
    const int nregion_bins = 10;  // fixme: this should come from the Response::Schema.
    const double halfwireextent = wire_pitch * 0.5 * (nwires - 1);
    cerr << "Max wire at pitch=" << halfwireextent << endl;

    std::vector<Pimpos> uvw_pimpos{
        Pimpos(nwires, -halfwireextent, halfwireextent, uwire, upitch, field_origin, nregion_bins),
        Pimpos(nwires, -halfwireextent, halfwireextent, vwire, vpitch, field_origin, nregion_bins),
        Pimpos(nwires, -halfwireextent, halfwireextent, wwire, wpitch, field_origin, nregion_bins)};

    // Digitization and time
    Binning tbins(nticks, t0, t0 + readout_time);

    // Diffusion
    const int ndiffision_sigma = 3.0;
    bool fluctuate = false;  // note, "point" negates this below

    // Generate some trivial tracks
    const double stepsize = 0.003 * units::mm;
    Gen::TrackDepos tracks(stepsize);

    // This is the number of ionized electrons for a MIP assumed by MB noise paper.
    // note: with option "point" this is overridden below.
    const double dqdx = 16000 * units::eplus / (3 * units::mm);
    const double charge_per_depo = -(dqdx) *stepsize;

    const double event_time = t0 + 1 * units::ms;
    const Point event_vertex(1.0 * units::m, 0 * units::m, 0 * units::mm);

    // mostly "prolonged" track in X direction
    if (track_types.find("prolong") < track_types.size()) {
        tracks.add_track(event_time,
                         Ray(event_vertex, event_vertex + Vector(1 * units::m, 0 * units::m, +10 * units::cm)),
                         charge_per_depo);
        tracks.add_track(event_time,
                         Ray(event_vertex, event_vertex + Vector(1 * units::m, 0 * units::m, -10 * units::cm)),
                         charge_per_depo);
    }

    // mostly "isochronous" track in Z direction, give spelling errors a break. :)
    if (track_types.find("isoch") < track_types.size()) {
        tracks.add_track(event_time, Ray(event_vertex, event_vertex + Vector(0, 0, 50 * units::mm)), charge_per_depo);
    }
    // "driftlike" track diagonal in space and drift time
    if (track_types.find("driftlike") < track_types.size()) {
        tracks.add_track(event_time,
                         Ray(event_vertex, event_vertex + Vector(60 * units::cm, 0 * units::m, 10.0 * units::mm)),
                         charge_per_depo);
    }

    // make a +
    if (track_types.find("plus") < track_types.size()) {
        tracks.add_track(event_time, Ray(event_vertex, event_vertex + Vector(0, 0, +1 * units::m)), charge_per_depo);
        tracks.add_track(event_time, Ray(event_vertex, event_vertex + Vector(0, 0, -1 * units::m)), charge_per_depo);
        tracks.add_track(event_time, Ray(event_vertex, event_vertex + Vector(0, +1 * units::m, 0)), charge_per_depo);
        tracks.add_track(event_time, Ray(event_vertex, event_vertex + Vector(0, -1 * units::m, 0)), charge_per_depo);
    }

    // // make a .
    if (track_types.find("point") < track_types.size()) {
        fluctuate = false;
        for (int i = 0; i < 6; i++) {
            auto vt = event_vertex + Vector(0, 0, i * 0.06 * units::mm);
            auto tt = event_time + i * 10.0 * units::us;
            tracks.add_track(tt, Ray(vt, vt + Vector(0, 0, 0.1 * stepsize)),  // force 1 point
                             -1.0 * units::eplus);
        }

        /* tracks.add_track(event_time, */
        /*                  Ray(event_vertex, */
        /*                      event_vertex + Vector(0, 0, 0.1*stepsize)), // force 1 point */
        /*                  -1.0*units::eplus); */
    }

    em("made tracks");

    // Get depos
    auto depos = tracks.depos();

    std::cerr << "got " << depos.size() << " depos from tracks\n";
    em("made depos");

    TFile* rootfile = TFile::Open(Form("%s-uvw.root", out_basename.c_str()), "recreate");
    TCanvas* canvas = new TCanvas("c", "canvas", 1000, 1000);
    gStyle->SetOptStat(0);

    std::string pdfname = argv[0];
    pdfname += ".pdf";
    canvas->Print((pdfname + "[").c_str(), "pdf");

    IRandom::pointer rng = nullptr;
    if (fluctuate) {
        rng = Factory::lookup<IRandom>("Random");
    }

    for (int plane_id = 0; plane_id < 3; ++plane_id) {
        em("start loop over planes");
        Pimpos& pimpos = uvw_pimpos[plane_id];

        // add deposition to binned diffusion
        Gen::BinnedDiffusion bindiff(
            pimpos, tbins, ndiffision_sigma, rng,
            Gen::BinnedDiffusion::ImpactDataCalculationStrategy::constant);  // default is constant interpolation
        em("made BinnedDiffusion");
        for (auto depo : depos) {
            auto drifted = std::make_shared<Gen::TransportedDepo>(depo, field_origin.x(), drift_speed);

            // In the real simulation these sigma are a function of
            // drift time.  Hard coded here with small values the
            // resulting voltage peak due to "point" source should
            // correspond to what is also shown on a per-impact
            // "Detector Response" from util's test_impactresponse.
            // Peak response of a delta function of current
            // integrating over time to one electron charge would give
            // 1eplus * 14mV/fC = 2.24 microvolt.
            const double sigma_time = 1 * units::us;
            const double sigma_pitch = 1.5 * units::mm;

            bool ok = bindiff.add(drifted, sigma_time, sigma_pitch);
            if (!ok) {
                std::cerr << "failed to add: t=" << drifted->time() / units::us << ", pt=" << drifted->pos() / units::mm
                          << std::endl;
            }
            Assert(ok);

            std::cerr << "depo:"
                      << " q=" << drifted->charge() / units::eplus << "ele"
                      << " time-T0=" << (drifted->time() - t0) / units::us << "us +/- " << sigma_time / units::us
                      << " us "
                      << " pt=" << drifted->pos() / units::mm << " mm\n";
        }
        em("added track depositions");

        auto ipir = Factory::find_tn<IPlaneImpactResponse>(pir_tns[plane_id]);

        em("looked up " + pir_tns[plane_id]);
        {
            const Response::Schema::PlaneResponse* pr = fr.plane(plane_id);
            const double pmax = 0.5 * ipir->pitch_range();
            const double pstep = std::abs(pr->paths[1].pitchpos - pr->paths[0].pitchpos);
            const int npbins = 2.0 * pmax / pstep;
            const int ntbins = pr->paths[0].current.size();

            const double tmin = fr.tstart;
            const double tmax = fr.tstart + fr.period * ntbins;
            TH2F* hpir = new TH2F(Form("hfr%d", plane_id), Form("Field Response %c-plane", uvw[plane_id]), ntbins, tmin,
                                  tmax, npbins, -pmax, pmax);
            for (auto& path : pr->paths) {
                const double cpitch = path.pitchpos;
                for (size_t ic = 0; ic < path.current.size(); ++ic) {
                    const double ctime = fr.tstart + ic * fr.period;
                    const double charge = path.current[ic] * fr.period;
                    hpir->Fill(ctime, cpitch, -1 * charge / units::eplus);
                }
            }
            hpir->SetZTitle("Induced charge [eles]");
            hpir->Write();

            hpir->Draw("colz");
            if (track_types.find("point") < track_types.size()) {
                hpir->GetXaxis()->SetRangeUser(70. * units::us, 100. * units::us);
                hpir->GetYaxis()->SetRangeUser(-10. * units::mm, 10. * units::mm);
            }
            canvas->Update();
            // canvas->Print(Form("%s_%c_resp.png", out_basename.c_str(), uvw[plane_id]));
            canvas->Print(pdfname.c_str(), "pdf");
        }
        em("wrote and leaked response hist");

        Gen::ImpactZipper zipper(ipir, bindiff);
        em("made ImpactZipper");

        // Set pitch range for plot y-axis
        auto rbins = pimpos.region_binning();
        auto pmm = bindiff.pitch_range(ndiffision_sigma);
        const int wbin0 = max(0, rbins.bin(pmm.first) - 40);
        const int wbinf = min(rbins.nbins() - 1, rbins.bin(pmm.second) + 40);
        const int nwbins = 1 + wbinf - wbin0;

        // Dead reckon
        const int tbin0 = 3500, tbinf = 5500;
        const int ntbins = tbinf - tbin0;

        std::map<int, Waveform::realseq_t> frame;
        double tottot = 0.0;
        for (int iwire = wbin0; iwire <= wbinf; ++iwire) {
            auto wave = zipper.waveform(iwire);
            auto tot = Waveform::sum(wave);
            if (tot != 0.0) {
                auto mm = std::minmax_element(wave.begin(), wave.end());
                cerr << "^ Wire " << iwire << " tot=" << tot / units::uV << " uV"
                     << " mm=[" << (*mm.first) / units::uV << "," << (*mm.second) / units::uV << "] uV " << endl;
            }

            tottot += tot;
            if (std::abs(iwire - 1000) <= 1) {  // central wires for "point"
                auto mm = std::minmax_element(wave.begin(), wave.end());
                std::cerr << "central wire: " << iwire << " mm=[" << (*mm.first) / units::microvolt << ","
                          << (*mm.second) / units::microvolt << "] uV\n";
            }
            frame[iwire] = wave;
        }
        em("zipped through wires");
        cerr << "Tottot = " << tottot << endl;
        Assert(tottot != 0.0);

        TH2F* hist = new TH2F(Form("h%d", plane_id), Form("Wire vs Tick %c-plane", uvw[plane_id]), ntbins, tbin0,
                              tbin0 + ntbins, nwbins, wbin0, wbin0 + nwbins);
        hist->SetXTitle("tick");
        hist->SetYTitle("wire");
        hist->SetZTitle("Voltage [-#muV]");

        std::cerr << nwbins << " wires: [" << wbin0 << "," << wbinf << "], " << ntbins << " ticks: [" << tbin0 << ","
                  << tbinf << "]\n";

        em("created TH2F");
        for (auto wire : frame) {
            const int iwire = wire.first;
            Assert(rbins.inbounds(iwire));
            const Waveform::realseq_t& wave = wire.second;
            // auto tot = Waveform::sum(wave);
            // std::cerr << iwire << " tot=" << tot << std::endl;
            for (int itick = tbin0; itick <= tbinf; ++itick) {
                hist->Fill(itick + 0.1, iwire + 0.1, -1.0 * wave[itick] / units::microvolt);
            }
        }

        if (track_types.find("point") < track_types.size()) {
            hist->GetXaxis()->SetRangeUser(3950, 4100);
            hist->GetYaxis()->SetRangeUser(996, 1004);
        }
        if (track_types.find("isoch") < track_types.size()) {
            hist->GetXaxis()->SetRangeUser(3900, 4000);
            hist->GetYaxis()->SetRangeUser(995, 1020);
        }
        em("filled TH2F");
        hist->Write();
        em("wrote TH2F");
        hist->Draw("colz");
        canvas->SetRightMargin(0.15);
        em("drew TH2F");
        std::vector<TLine*> lines;
        auto trqs = tracks.tracks();
        for (size_t iline = 0; iline < trqs.size(); ++iline) {
            auto trq = trqs[iline];
            const double time = get<0>(trq);
            const Ray ray = get<1>(trq);

            // this need to subtract off the fr.origin is I think a bug,
            // or at least a bookkeeping detail to ensconce somewhere.  I
            // think FR is taking the start of the path as the time
            // origin.  Something to check...
            const int tick1 = tbins.bin(time + (ray.first.x() - fr.origin) / drift_speed);
            const int tick2 = tbins.bin(time + (ray.second.x() - fr.origin) / drift_speed);

            const int wire1 = rbins.bin(pimpos.distance(ray.first));
            const int wire2 = rbins.bin(pimpos.distance(ray.second));

            cerr << "digitrack: t=" << time << " ticks=[" << tick1 << "," << tick2 << "] wires=[" << wire1 << ","
                 << wire2 << "]\n";

            const int fudge = 0;
            TLine* line = new TLine(tick1 - fudge, wire1, tick2 - fudge, wire2);
            line->Write(Form("l%c%d", uvw[plane_id], (int) iline));
            line->Draw();
            // canvas->Print(Form("%s_%c.png", out_basename.c_str(), uvw[plane_id]));
            canvas->Print(pdfname.c_str(), "pdf");
        }
        em("printed PNG canvases");
        em("end of PIR scope");

        // canvas->Print("test_impactzipper.pdf","pdf");
    }
    rootfile->Close();
    canvas->Print((pdfname + "]").c_str(), "pdf");
    em("done");

    // cerr << em.summary() << endl;
    return 0;
}
