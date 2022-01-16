#ifndef WIRECELLUTIL_FFTBESTLENGTH_H
#define WIRECELLUTIL_FFTBESTLENGTH_H

#include <cstddef>

namespace WireCell {
    // Return suggested number of samples for performing an FFT which
    // should have no worse performance than the input nsamples.
    std::size_t fft_best_length(size_t nsamples, bool keep_odd_even = false);
}  // namespace WireCell

#endif
