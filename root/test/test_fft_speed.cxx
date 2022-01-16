#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Array.h"

#include "TCanvas.h"
#include "TH1F.h"
#include "TLine.h"
#include "TText.h"
#include "TGraph.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <complex>

#include <fstream>
#include <sstream>

using namespace std;
using namespace WireCell;

// The preferred display units for gain.
const double GUnit = units::mV / units::fC;

int main(int argc, char** argv)
{
    int nInputs = 0;
    int nBegin = 0;
    int nEnd = 0;
    int id = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            stringstream convert(argv[i + 1]);
            if (!(convert >> nInputs)) {
                cerr << " ---> Error Scan !" << endl;
                exit(0);
            }
        }

        if (strcmp(argv[i], "-b") == 0) {
            stringstream convert(argv[i + 1]);
            if (!(convert >> nBegin)) {
                cerr << " ---> Error Scan !" << endl;
                exit(0);
            }
        }

        if (strcmp(argv[i], "-e") == 0) {
            stringstream convert(argv[i + 1]);
            if (!(convert >> nEnd)) {
                cerr << " ---> Error Scan !" << endl;
                exit(0);
            }
        }

        if (strcmp(argv[i], "-id") == 0) {
            stringstream convert(argv[i + 1]);
            if (!(convert >> id)) {
                cerr << " ---> Error Scan !" << endl;
                exit(0);
            }
        }
    }

    if (nInputs == 0 || nBegin == 0 || nEnd == 0 || id == 0) {
        cerr << endl
             << " Usage: " << endl
             << "\t"
             << "./test_scan_fft -id ID_Num -n NumA -b NumB -e NumC" << endl
             << endl;
    }

    cout << TString::Format(" ---> nInputs %d, %d, %d", nInputs, nBegin, nEnd) << endl;

    const std::vector<double> gains = {7.8 * GUnit, 14.0 * GUnit};
    const std::vector<double> shapings = {1.0 * units::us, 2.0 * units::us};

    const double maxtime = 100.0 * units::us;
    const double tick = 0.5 * units::us;
    const Binning tbins(maxtime / tick, 0, maxtime);
    const int nticks = tbins.nbins();
    cerr << "Using nticks=" << nticks << endl;

    // do timing tests
    {
        cout << endl
             << " nSamples, fwd_time, fwd_time per sample, rev_time, rev_time per sample, average time of (fwd and rev)"
             << endl
             << endl;

        ofstream output(TString::Format("%s_%02d.txt", argv[0], id), ios::out | ios::trunc);

        std::vector<int> nsampleslist;

        for (int i = 0; i < nInputs; i++) {
            int nsamples = nBegin + i;
            if (nsamples > nEnd) break;
            nsampleslist.push_back(nsamples);
        }

        const int ntries = 1000;
        for (auto nsamps : nsampleslist) {
            // Response::ColdElec ce(gains[1], shapings[1]);
            // const Binning bins(nsamps, 0, maxtime);
            // Waveform::realseq_t res = ce.generate(bins);
            // Waveform::compseq_t spec;
            Array::array_xxc test_array = Array::array_xxc::Zero(ntries, nsamps);
            for (int i = 0; i != ntries; i++) {
                for (int j = 0; j != nsamps; j++) {
                    test_array(i, j) = std::complex<float>(1.1, 1.3);
                }
            }

            double fwd_time = 0.0;
            // for (int itry=0; itry<ntries; ++itry) {
            // 	auto t1 = std::chrono::high_resolution_clock::now();
            // 	spec = Waveform::dft(res);
            // 	auto t2 = std::chrono::high_resolution_clock::now();
            // 	fwd_time += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
            // }
            // fwd_time /= ntries;

            auto t1 = std::chrono::high_resolution_clock::now();
            Array::dft_cc(test_array, 0);
            auto t2 = std::chrono::high_resolution_clock::now();
            fwd_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / ntries;

            double rev_time = 0.0;
            // for (int itry=0; itry<ntries; ++itry) {
            auto t3 = std::chrono::high_resolution_clock::now();
            // 	res = Waveform::idft(spec);
            Array::idft_cc(test_array, 0);
            auto t4 = std::chrono::high_resolution_clock::now();
            rev_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count() / ntries;
            // }
            // rev_time /= ntries;

            // cout<<TString::Format(" DFT nsamples= %5d, tforward: %7.2f us, %9.6f us/sample, treverse: %7.2f us, %9.6f
            // us/sample, taverage: %7.2f us", 			    nsamps, 			    fwd_time/1000., fwd_time/nsamps/1000., 			    rev_time/1000.,
            // rev_time/nsamps/1000., 			    0.5*(fwd_time+rev_time)/1000.
            // 			    )<<endl;

            output << TString::Format("%5d %7.2f %9.6f %7.2f %9.6f %7.2f", nsamps, fwd_time / 1000.,
                                      fwd_time / nsamps / 1000., rev_time / 1000., rev_time / nsamps / 1000.,
                                      0.5 * (fwd_time + rev_time) / 1000.)
                   << endl;

        }  // for (auto nsamps : nsampleslist) {

        output.close();
    }

    cout << endl << " Complete " << endl << endl;

}  // main
