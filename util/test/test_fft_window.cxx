#include "WireCellUtil/FFTBestLength.h"

#include <iostream>
#include <vector>

using namespace WireCell;

int main(int argc, char** argv)
{
    std::vector<std::size_t> lengths{2,     4,    8,    16,   32,
                                     64,    128,  256,  400,  480,  // protoDUNE U/V and W channels per plane
                                     512,
                                     800,  // protoDUNE, sum of U or V channels for both faces
                                     960,  // protoDUNE, sum of W channels (or wires) for both faces
                                     1024,
                                     1148,  // N wires in U/V plane for protodune
                                     2048,
                                     2400,  // number of channels in U or V in microboone
                                     2560,  // DUNE, total APA channels
                                     3456,  // number of channels in microboone's W
                                     4096,
                                     6000,  // one choice of nticks for protoDUNE
                                     8192,
                                     8256,                     // total microboone channels
                                     9592,  9594, 9595, 9600,  // various microboone readout lengths
                                     9892,                     // ?
                                     10000,                    // 5 ms at 2MHz readout
                                     10240, 16384};

    for (auto len : lengths) {
        for (auto add : {-1, 0, 1}) {
            const size_t ini = len + add;
            const size_t t = fft_best_length(ini, true);
            const size_t f = fft_best_length(ini, false);
            std::cout << ini << " " << t << " " << f << std::endl;
        }
    }
    std::cout << "<num> <keep=true> <keep=false>\n";
    return 0;
}
