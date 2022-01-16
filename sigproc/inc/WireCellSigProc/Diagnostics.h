#ifndef WIRECELLSIGPROC_DIAGNOSTICS
#define WIRECELLSIGPROC_DIAGNOSTICS

#include "WireCellUtil/Waveform.h"

namespace WireCell {
    namespace SigProc {
        namespace Diagnostics {

            // Functional object to find the first chirp region in the signal.
            class Chirp {
                const int windowSize;
                const double chirpMinRMS, maxNormalNeighborFrac;

               public:
                /// Create a chirp detector using magic numbers.
                Chirp(int windowSize = 20, double chirpMinRMS = 0.9, double maxNormalNeighborFrac = 0.20);

                // Return true if a chirp region is found and set beg/end indices to its half-open bounds.
                bool operator()(const WireCell::Waveform::realseq_t& sig, int& beg, int& end) const;
            };

            // Return true if DC component dominates over nfreqs lowest
            // frequencies and if average low frequency power is more than
            // maxpower.
            // bool id_rc(const WireCell::Waveform::compseq_t& spec, int nfreqs=4, float maxpower=6000.0);
            class Partial {
                const int nfreqs;
                const double maxpower;

               public:
                /// Create a partial waveform detector using magic numbers.
                Partial(int nfreqs = 4, double maxpower = 6000.0);

                /// Return true if given frequency spectrum consistent with a partial waveform
                bool operator()(const WireCell::Waveform::compseq_t& spec) const;
            };

        }  // namespace Diagnostics
    }      // namespace SigProc
}  // namespace WireCell

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
