#include "WireCellSigProc/Derivations.h"

#include <iostream>

using namespace WireCell::SigProc;

std::pair<double, double> Derivations::CalcRMS(const WireCell::Waveform::realseq_t& signal)
{
    std::pair<double, double> temp = WireCell::Waveform::mean_rms(signal);
    double mean = temp.first;
    double rms = temp.second;
    WireCell::Waveform::realseq_t temp1;
    for (size_t i = 0; i != signal.size(); i++) {
        if (fabs(signal.at(i) - mean) < 4.5 * rms) {
            temp1.push_back(signal.at(i));
        }
    }
    temp = WireCell::Waveform::mean_rms(temp1);
    return temp;
}

WireCell::Waveform::realseq_t Derivations::CalcMedian(const WireCell::IChannelFilter::channel_signals_t& chansig)
{
    float max_rms = 0;
    float count_max_rms = 0;
    const int nchannel = chansig.size();
    const int nbins = (chansig.begin()->second).size();
    // float content[nchannel][nbins];
    std::vector<float> content(nchannel * nbins, 0.0);  // 2D array [channel*nbins + bin]

    int start_ch = 0;
    for (auto it : chansig) {
        // const int ch = it.first;
        WireCell::IChannelFilter::signal_t& signal = it.second;
        std::pair<double, double> temp = WireCell::Waveform::mean_rms(signal);

        if (temp.second > 0) {
            max_rms += temp.second;
            count_max_rms++;
        }

        for (int i = 0; i != nbins; i++) {
            content[start_ch * nbins + i] = signal.at(i);
        }
        start_ch++;
    }

    if (count_max_rms > 0) {
        max_rms /= count_max_rms;
    }

    WireCell::Waveform::realseq_t medians(nbins);
    for (int ibin = 0; ibin != nbins; ibin++) {
        WireCell::Waveform::realseq_t temp;
        for (int ich = 0; ich != nchannel; ich++) {
            const float cont = content.at(ich * nbins + ibin);
            if (fabs(cont) < 5 * max_rms && fabs(cont) > 0.001) {
                temp.push_back(cont);
            }
        }
        if (temp.size() > 0) {
            medians.at(ibin) = WireCell::Waveform::median_binned(temp);
        }
        else {
            medians.at(ibin) = 0;
        }
    }

    return medians;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
