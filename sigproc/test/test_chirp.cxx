#include "WireCellSigProc/Diagnostics.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <string>

// provides vectors "horig" and "hfilt"
// generate like:
// $ dump-root-hist-to-waveform sigproc/test/example-chirp.root horig hfilt > sigproc/test/example-chirp.h
#include <vector>
#include "example-chirp.h"

using namespace std;

using namespace WireCell;
using namespace WireCell::SigProc;

int main(int argc, char* argv[])
{
    // vectors are in example-chirp.h

    int beg = -1, end = -1;
    Diagnostics::Chirp chirp;
    bool found = chirp(horig, beg, end);
    Assert(found);

    // the function should find something starting at the beginning.
    Assert(beg == 0);
    // and the chirp does not extend to the end
    Assert(end != 0 && end != (int) horig.size());

    Assert(beg >= 0);
    Assert(end >= 0);
    Assert(beg < end);

    // The algorithm works in chunks of 20
    Assert((end - beg) % 20 == 0);

    for (int ind = beg; ind < end; ++ind) {
        Assert(hfilt[ind] == 0);
    }

    cerr << "chirp at " << beg << " --> " << end << endl;
    Assert(end == 4240);

    return 0;
}
