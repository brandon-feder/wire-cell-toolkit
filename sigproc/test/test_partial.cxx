#include "WireCellSigProc/Diagnostics.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <string>

// provides vectors "horig" and "hfilt"
#include <vector>
#include "example-partial-rc.h"

using namespace std;

using namespace WireCell;
using namespace WireCell::SigProc;

int main(int argc, char* argv[])
{
    auto spectrum = Waveform::dft(horig);
    Diagnostics::Partial m_check_partial;
    bool is_partial = m_check_partial(spectrum);
    Assert(is_partial);

    return 0;
}
