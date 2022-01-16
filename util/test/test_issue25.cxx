#include "WireCellUtil/Waveform.h"

#include <iostream>
#include <chrono>
#include <random>

using namespace std;
using namespace WireCell::Waveform;

int main()
{
    const std::vector<float> percentiles{0.25, 0.5, 0.75};

    const size_t wfsize = 10000;
    random_device rnd_device;
    mt19937 mersenne_engine{rnd_device()};
    uniform_real_distribution<float> dist{0, 4096};
    realseq_t orig(wfsize);
    for (size_t ind = 0; ind < wfsize; ++ind) {
        orig[ind] = dist(mersenne_engine);
    }

    double ref = 0;
    {
        double tt = 0;
        int count = 1000;
        while (count) {
            --count;
            realseq_t wfvec = orig;

            auto t1 = std::chrono::high_resolution_clock::now();
            for (float pt : percentiles) {
                percentile(wfvec, pt);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tt += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        }
        cerr << 1e-9 * tt << " (no copy: 1.0)" << endl;
        ref = tt;
    }

    {
        double tt = 0;
        int count = 1000;
        while (count) {
            --count;
            realseq_t wfvec = orig;

            auto t1 = std::chrono::high_resolution_clock::now();
            for (float pt : percentiles) {
                realseq_t copy = wfvec;
                percentile(copy, pt);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tt += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        }
        cerr << 1e-9 * tt << " (copy timed: " << tt / ref << ")" << endl;
    }
    {
        double tt = 0;
        int count = 1000;
        while (count) {
            --count;
            realseq_t wfvec = orig;

            for (float pt : percentiles) {
                realseq_t copy = wfvec;
                auto t1 = std::chrono::high_resolution_clock::now();
                percentile(copy, pt);
                auto t2 = std::chrono::high_resolution_clock::now();
                tt += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
            }
        }
        cerr << 1e-9 * tt << " (copy not timed: " << tt / ref << ")" << endl;
    }
    return 0;
}
