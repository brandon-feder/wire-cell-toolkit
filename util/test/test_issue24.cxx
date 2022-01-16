#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

using namespace std;

using namespace WireCell::Waveform;
using namespace WireCell;

int main()
{
    int nsamples = 10;
    while (nsamples > 0) {
        realseq_t wave(nsamples, 0);
        median(wave);
        --nsamples;
    }

    cerr << "Testing error handling\n";
    realseq_t wave;
    bool okay = false;
    try {
        median(wave);
    }
    catch (ValueError& err) {
        okay = true;
        cerr << "Caught:\n" << err.what() << "\nOKAY\n";
    }
    catch (std::exception& err) {
        cerr << "Why am I here?\n";
        cerr << err.what() << "\n";
    }
    if (!okay) {
        cerr << "median of empty wave should throw\n";
    }
    assert(okay);
    cerr << "thrown and caught empty waveform\n";

    wave.push_back(6.9);
    wave.push_back(9.6);
    okay = false;
    try {
        percentile(wave, -0.1);
    }
    catch (ValueError& err) {
        okay = true;
        cerr << "Caught:\n" << err.what() << "\nOKAY\n";
    }
    if (!okay) {
        cerr << "median under percentage should throw\n";
    }
    assert(okay);
    cerr << "thrown and caught median under percentage\n";
    
    okay = false;
    try {
        percentile(wave, 1.1);
    }
    catch (ValueError& err) {
        okay = true;
        cerr << "Caught:\n" << err.what() << "\nOKAY\n";
    }
    if (!okay) {
        cerr << "median over percentage should throw\n";
    }
    assert(okay);
    cerr << "thrown and caught median over percentage\n";

    cerr << median(wave) << endl;
    assert(std::abs(9.6 - median(wave)) < 0.001);
    wave.push_back(0.0);
    cerr << median(wave) << endl;
    assert(std::abs(6.9 - median(wave)) < 0.001);
    wave.push_back(10.0);
    cerr << median(wave) << endl;
    assert(std::abs(9.6 - median(wave)) < 0.001);

    return 0;
}
