// This is some "private" code shared by a couple of components in gen.
//
// fixme: this is a candidate for turning into an interface.

#include "WireCellIface/IRandom.h"
#include "WireCellUtil/Waveform.h"

#include <vector>

namespace WireCell {
    namespace Gen {
        namespace Noise {
            // Generate a time series waveform given a spectral amplitude
            WireCell::Waveform::realseq_t generate_waveform(const std::vector<float>& spec, IRandom::pointer rng,
                                                            double replace = 0.02);
        }  // namespace Noise
    }      // namespace Gen
}  // namespace WireCell
