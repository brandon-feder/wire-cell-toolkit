#include "WireCellUtil/Waveform.h"
#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    {
        std::vector<short> adcv{-1, 0, 0, 1, 1, 2, 3, 4, 4, 4, 5, 6, 4, 3, 4, 5, 6, 7, 7, 6, 5};
        auto mf = Waveform::most_frequent(adcv);
        cerr << mf << endl;
    }
    {
        std::vector<short> adcv{-1, 0, 0, -1, -1, 2, 3, 4, 4, -1, -1, 5, 6, 4, 3, 4, 5, 6, 7, 7, 6, 5};
        auto mf = Waveform::most_frequent(adcv);
        cerr << mf << endl;
    }
    {
        std::vector<short> adcv{5, 5, 5, 5, 1, 1, 1, 2, 2, 2, 2};
        auto mf = Waveform::most_frequent(adcv);
        cerr << mf << endl;
    }
}
