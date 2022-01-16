#include "WireCellSigProc/Diagnostics.h"
#include "WireCellSigProc/Microboone.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <string>

// provides vectors "horig" and "hfilt"
// generate like:
// $ dump-root-hist-to-waveform sigproc/test/example-chirp.root horig hfilt > sigproc/test/example-chirp.h
#include <vector>
#include "example-noisy.h"

using namespace std;

using namespace WireCell;
using namespace WireCell::SigProc;

int main(int argc, char* argv[])
{
    Microboone::SignalFilter(horig);
    bool is_noisy = Microboone::NoisyFilterAlg(horig, 0.7, 10.0);
    Assert(is_noisy);
}
