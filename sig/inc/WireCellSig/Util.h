#ifndef WIRECELLSIG_UTIL
#define WIRECELLSIG_UTIL

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Array.h"

namespace WireCell {
    namespace Sig {
        void restore_baseline(Array::array_xxf& arr);
    }
}  // namespace WireCell

#endif