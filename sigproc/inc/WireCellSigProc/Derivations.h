#ifndef WIRECELLSIGPROC_DERIVATIONS
#define WIRECELLSIGPROC_DERIVATIONS

#include "WireCellUtil/Waveform.h"
#include "WireCellIface/IChannelFilter.h"

namespace WireCell {
    namespace SigProc {
        namespace Derivations {

            /** Return a sequence of the median values, one for each channel signal.

                fixme and warning: this ignores small and large signal
                with fixed thresholds which need to be set as parameters.
            */
            WireCell::Waveform::realseq_t CalcMedian(const WireCell::IChannelFilter::channel_signals_t& chansig);

            /** Return the RMS of a signal

                fixme and warning: this ignores large signal with a fixed
                threshold which needs to be set as parameters.
            */
            std::pair<double, double> CalcRMS(const WireCell::Waveform::realseq_t& signal);
        }  // namespace Derivations
    }      // namespace SigProc
}  // namespace WireCell

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
