#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Logging.h"

/// needed to pretend like we are doing WCT internals
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IFieldResponse.h"
#include "WireCellIface/IConfigurable.h"

#include "TFile.h"
#include "TGraph.h"

#include <iostream>

using namespace WireCell;
using namespace std;
using spdlog::debug;
using spdlog::error;

int main(int argc, char* argv[])
{
    Log::add_stdout(true, "debug");
    Log::set_level("debug");

    std::string frfname = "ub-10-wnormed.json.bz2";
    if (argc > 1) {
        frfname = argv[1];
        cerr << "Using command line field response file: " << frfname << endl;
    }
    else {
        cerr << "Using default field response file: " << frfname << endl;
    }

    /// WCT internals, normally user code does not need this
    {
        PluginManager& pm = PluginManager::instance();
        pm.add("WireCellSigProc");
        auto ifrcfg = Factory::lookup<IConfigurable>("FieldResponse");
        auto cfg = ifrcfg->default_configuration();
        cfg["filename"] = frfname;
        ifrcfg->configure(cfg);
    }

    auto ifr = Factory::find<IFieldResponse>("FieldResponse");

    // Get full, "fine-grained" field responses defined at impact
    // positions.
    Response::Schema::FieldResponse fr = ifr->field_response();

    cerr << "FR with " << fr.planes[0].paths.size() << " responses per plane\n";
    Assert(fr.planes[0].paths.size() == 21 * 6);

    for (auto pr : fr.planes) {
        const int n_per = 6;
        const double imp_pitch = 2.0 * std::abs(pr.paths[n_per - 1].pitchpos - pr.paths[0].pitchpos);
        debug("plane:{}, pitch:{:f}, pitchpos: front:{:f}, back:{:f}, imp-pitch:{:f}",
              pr.planeid, pr.pitch,
              pr.paths.front().pitchpos,
              pr.paths.back().pitchpos,
              imp_pitch
            );
        // for (auto path : pr.paths) {
        //     debug("\tpitchpos:{}", path.pitchpos);
        // }
    }


    // Make a new data set which is the average FR
    Response::Schema::FieldResponse fravg = Response::wire_region_average(fr);

    cerr << "FR with " << fravg.planes[0].paths.size() << " responses per plane\n";
    /// fixme: why is this is producing 22 responses per plane and not 21?

    double period = 0.5 * units::microsecond;

    // convolute with electronics response function
    WireCell::Waveform::compseq_t elec;
    WireCell::Binning tbins(Response::as_array(fravg.planes[0]).cols(), 0,
                            Response::as_array(fravg.planes[0]).cols() * fravg.period);
    Response::ColdElec ce(14.0 * units::mV / units::fC, 2.0 * units::microsecond);
    auto ewave = ce.generate(tbins);
    Waveform::scale(ewave, 1.2 * 4096 / 2000.);
    elec = Waveform::dft(ewave);

    std::complex<float> fine_period(fravg.period, 0);

    // TFile *file = new TFile("temp1.root","RECREATE");
    // TGraph **gu = new TGraph*[21];
    // TGraph **gv = new TGraph*[21];
    // TGraph **gw = new TGraph*[21];
    // TGraph *gtemp;

    Waveform::realseq_t wfs(9594);
    Waveform::realseq_t ctbins(9594);
    for (int i = 0; i != 9594; i++) {
        ctbins.at(i) = i * period;
    }
    Waveform::realseq_t ftbins(Response::as_array(fravg.planes[0]).cols());
    for (int i = 0; i != Response::as_array(fravg.planes[0]).cols(); i++) {
        ftbins.at(i) = i * fravg.period;
    }

    // Convert each average FR to a 2D array
    for (int ind = 0; ind < 3; ++ind) {
        auto arr = Response::as_array(fravg.planes[ind]);

        // do FFT for response ...
        Array::array_xxc c_data = Array::dft_rc(arr, 0);
        int nrows = c_data.rows();
        int ncols = c_data.cols();

        for (int irow = 0; irow < nrows; ++irow) {
            for (int icol = 0; icol < ncols; ++icol) {
                c_data(irow, icol) = c_data(irow, icol) * elec.at(icol) * fine_period;
            }
        }

        arr = Array::idft_cr(c_data, 0);

        // figure out how to do fine ... shift (good ...)
        auto arr1 = arr.block(0, 0, nrows, 100);
        arr.block(0, 0, nrows, ncols - 100) = arr.block(0, 100, nrows, ncols - 100);
        arr.block(0, ncols - 100, nrows, 100) = arr1;

        // redigitize ...
        for (int irow = 0; irow < nrows; ++irow) {
            // gtemp = new TGraph();

            int fcount = 1;
            for (int i = 0; i != 9594; i++) {
                double ctime = ctbins.at(i);

                if (fcount < 1000)
                    while (ctime > ftbins.at(fcount)) {
                        fcount++;
                        if (fcount >= 1000) break;
                    }

                if (fcount < 1000) {
                    // interpolate between fbins.at(fcount - 1) and fbins.at(fcount)
                    wfs.at(i) = (ctime - ftbins.at(fcount - 1)) / fravg.period * arr(irow, fcount - 1) +
                                (ftbins.at(fcount) - ctime) / fravg.period * arr(irow, fcount);
                }
                else {
                    wfs.at(i) = 0;
                }

                // gtemp->SetPoint(i,ctime/units::microsecond,wfs.at(i)/units::mV*(-1));
            }

            // if (ind ==0){
            //   gu[irow] = gtemp;
            //   //	    for (int icol=0; icol < ncols; ++ icol){
            //   //  gu[irow]->SetPoint(icol,icol*0.1,arr(irow,icol)/units::mV*(-1));
            //   // }
            // }else if (ind==1){
            //   gv[irow] = gtemp;
            //   // for (int icol=0; icol < ncols; ++ icol){
            //   //   gv[irow]->SetPoint(icol,icol*0.1,arr(irow,icol)/units::mV*(-1));
            //   // }
            // }else if (ind==2){
            //   gw[irow] = gtemp;
            //   // for (int icol=0; icol < ncols; ++ icol){
            //   //   gw[irow]->SetPoint(icol,icol*0.1,arr(irow,icol)/units::mV*(-1));
            //   // }
            // }
        }

        // cerr << "FRavg: plane " << ind << ": " << arr.rows() << " X " << arr.cols() << " " <<
        // fravg.period/units::microsecond << endl;
    }

    // file->cd();

    // for (int i=0;i!=21;i++){
    //   gu[i]->Write(Form("gu_%d",i));
    //   gv[i]->Write(Form("gv_%d",i));
    //   gw[i]->Write(Form("gw_%d",i));
    // }
    // file->Close();

    // for (size_t i=0;i!=ewave.size();i++){
    //   std::cout << i *0.1 << " " << ewave.at(i) / (units::mV/units::fC) << std::endl;
    // }
}
