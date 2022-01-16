#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <algorithm>
#include <complex>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using namespace WireCell;

void test_transform()
{
    const int baseline = 1000;
    const int nticks = 100;
    const int period = 10;

    Waveform::realseq_t wf1(nticks), wf2(nticks);

    for (int ind = 0; ind < nticks; ++ind) {
        wf1[ind] = baseline + ind % period;
    }

    transform(wf1.begin(), wf1.end(), wf2.begin(), [](int x) -> int { return x - baseline; });

    for (int ind = 0; ind < nticks; ++ind) {
        Assert(wf1[ind] - baseline == wf2[ind]);
    }
}

void test_mean_rms()
{
    Waveform::realseq_t v{1.0, 1.0, 2.0, 3.0, 4.0, 4.0, 4.0, 3.0};

    auto us = Waveform::mean_rms(v);
    auto m = Waveform::median(v);

    cerr << us.first << " +/- " << us.second << " med=" << m << endl;
}

void test_fft()
{
    Waveform::realseq_t s;
    const int nbins = 360;
    for (int ind = 0; ind < nbins; ++ind) {
        double phi = ind * 3.1415 / 180.0;
        Waveform::real_t val = sin(phi) + sin(11.0 / 7.0 * phi);
        s.push_back(val);
    }

    auto spec = Waveform::dft(s);
    for (int ind = 0; ind < nbins; ++ind) {
        auto c = spec[ind];
        cerr << ind << "\ts=" << s[ind] << "\tc=" << c << "\tmag=" << std::abs(c) << "\tphi=" << std::arg(c) << endl;
    }
    cerr << s.size() << " " << spec.size() << endl;
}

void test_complex()
{
    Waveform::compseq_t cv{{1.1, 2.2}, {-3.3, 4.4}, {0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    auto bogus = Waveform::idft(cv);
}

void test_arithmetic()
{
    using namespace WireCell::Waveform;
    realseq_t v{0.0, 1.0, 2.0};
    auto v2 = v;

    increase(v, 2.0);
    Assert(v[0] == 2.0);
    Assert(v[1] == 3.0);
    Assert(v[2] == 4.0);
    increase(v2, v);
    Assert(v2[0] = 2.0);
    Assert(v2[0] = 4.0);
    Assert(v2[0] = 6.0);

    scale(v, 2.0);
    scale(v2, v);

    compseq_t cv{{1.1, 2.2}, {-3.3, 4.4}, {0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    auto cv2 = cv;

    increase(cv, complex_t(1.0, 0.0));
    increase(cv2, cv);
    scale(cv, complex_t(1.0, 0.0));
    scale(cv2, cv);
}

int main(int argc, char* argv[])
{
    test_transform();
    test_fft();
    test_complex();
    test_mean_rms();
    test_arithmetic();

    cerr << "bye." << endl;
    return 0;
}
