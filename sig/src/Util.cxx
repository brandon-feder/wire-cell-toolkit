#include "WireCellSig/Util.h"

using namespace WireCell;

void Sig::restore_baseline(Array::array_xxf& arr)
{
    for (int i = 0; i != arr.rows(); i++) {
        Waveform::realseq_t signal(arr.cols());
        int ncount = 0;
        for (int j = 0; j != arr.cols(); j++) {
            if (arr(i, j) != 0) {
                signal.at(ncount) = arr(i, j);
                ncount++;
            }
        }
        signal.resize(ncount);
        float baseline = WireCell::Waveform::median(signal);

        Waveform::realseq_t temp_signal(arr.cols());
        ncount = 0;
        for (size_t j = 0; j != signal.size(); j++) {
            if (fabs(signal.at(j) - baseline) < 500) {
                temp_signal.at(ncount) = signal.at(j);
                ncount++;
            }
        }
        temp_signal.resize(ncount);

        baseline = WireCell::Waveform::median(temp_signal);

        for (int j = 0; j != arr.cols(); j++) {
            if (arr(i, j) != 0) arr(i, j) -= baseline;
        }
    }
}