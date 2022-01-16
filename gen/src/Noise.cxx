// This was chopped out of NoiseSource, originally by Xin.

#include "Noise.h"

using namespace WireCell;

Waveform::realseq_t Gen::Noise::generate_waveform(const std::vector<float>& spec, IRandom::pointer rng, double replace)
{
    // reuse randomes a bit to optimize speed.
    static std::vector<double> random_real_part;
    static std::vector<double> random_imag_part;

    if (random_real_part.size() != spec.size()) {
        random_real_part.resize(spec.size(), 0);
        random_imag_part.resize(spec.size(), 0);
        for (unsigned int i = 0; i < spec.size(); i++) {
            random_real_part.at(i) = rng->normal(0, 1);
            random_imag_part.at(i) = rng->normal(0, 1);
        }
    }
    else {
        const int shift1 = rng->uniform(0, random_real_part.size());
        // replace certain percentage of the random number
        const int step = 1. / replace;
        for (int i = shift1; i < shift1 + int(spec.size()); i += step) {
            if (i < int(spec.size())) {
                random_real_part.at(i) = rng->normal(0, 1);
                random_imag_part.at(i) = rng->normal(0, 1);
            }
            else {
                random_real_part.at(i - spec.size()) = rng->normal(0, 1);
                random_imag_part.at(i - spec.size()) = rng->normal(0, 1);
            }
        }
    }

    const int shift = rng->uniform(0, random_real_part.size());

    WireCell::Waveform::compseq_t noise_freq(spec.size(), 0);

    for (int i = shift; i < int(spec.size()); i++) {
        const double amplitude = spec.at(i - shift) * sqrt(2. / 3.1415926);  // / units::mV;
        noise_freq.at(i - shift).real(random_real_part.at(i) * amplitude);
        noise_freq.at(i - shift).imag(random_imag_part.at(i) * amplitude);  //= complex_t(real_part,imag_part);
    }
    for (int i = 0; i < shift; i++) {
        const double amplitude = spec.at(i + int(spec.size()) - shift) * sqrt(2. / 3.1415926);
        noise_freq.at(i + int(spec.size()) - shift).real(random_real_part.at(i) * amplitude);
        noise_freq.at(i + int(spec.size()) - shift).imag(random_imag_part.at(i) * amplitude);
    }

    Waveform::realseq_t noise_time = WireCell::Waveform::idft(noise_freq);
    return noise_time;
}
