#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Exceptions.h"

#include <algorithm>

// for FFT
#include <Eigen/Core>
#include <unsupported/Eigen/FFT>

#include <complex>

using namespace WireCell;

std::pair<int, int> WireCell::Waveform::sub_sample(const Waveform::Domain& domain, int nsamples,
                                                   const Waveform::Domain& subdomain)
{
    const double bin = sample_width(domain, nsamples);
    int beg = ceil((subdomain.first - domain.first) / bin);
    int end = nsamples - ceil((domain.second - subdomain.second) / bin);
    return std::make_pair(std::max(0, beg), std::min(nsamples, end));
}

std::pair<double, double> WireCell::Waveform::mean_rms(const realseq_t& wf)
{
    const int n = wf.size();
    if (n == 0) {
        return std::make_pair<double, double>(0, 0);
    }
    if (n == 1) {
        return std::make_pair<double, double>(wf[0], 0);
    }

    // if left as float, numerical precision will lead to many NaN for
    // the RMS due to sometimes subtracting similar sized numbers.
    std::vector<double> wfd(wf.begin(), wf.end());

    const double wsum = Waveform::sum(wfd);
    const double w2sum = Waveform::sum2(wfd);
    const double mean = wsum / n;
    const double rms = sqrt((w2sum - wsum * wsum / n) / n);
    //   const double rms = sqrt( (w2sum - wsum*wsum/n) / (n-1) );
    return std::make_pair(mean, rms);
}

template <class Func>
Waveform::realseq_t c2r(const Waveform::compseq_t& seq, Func func)
{
    Waveform::realseq_t ret(seq.size());
    std::transform(seq.begin(), seq.end(), ret.begin(), func);
    return ret;
}

Waveform::realseq_t WireCell::Waveform::real(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::real(c); });
}

Waveform::realseq_t WireCell::Waveform::imag(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::imag(c); });
}

Waveform::realseq_t WireCell::Waveform::magnitude(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::abs(c); });
}

Waveform::realseq_t WireCell::Waveform::phase(const Waveform::compseq_t& seq)
{
    return c2r(seq, [](Waveform::complex_t c) { return std::arg(c); });
}


Waveform::compseq_t Waveform::complex(const Waveform::realseq_t& real)
{
    Waveform::realseq_t imag(real.size(), 0);
    return Waveform::complex(real, imag);
}

Waveform::compseq_t Waveform::complex(const Waveform::realseq_t& real, const Waveform::realseq_t& imag)
{
    Waveform::compseq_t ret(real.size());
    std::transform(real.begin(), real.end(), imag.begin(), ret.begin(),
                   [](real_t re, real_t im) { return Waveform::complex_t(re,im); } );
    return ret;
}


Waveform::real_t WireCell::Waveform::median(Waveform::realseq_t& wave) { return percentile(wave, 0.5); }

Waveform::real_t WireCell::Waveform::median_binned(Waveform::realseq_t& wave) { return percentile_binned(wave, 0.5); }

Waveform::real_t WireCell::Waveform::percentile(Waveform::realseq_t& wave, real_t percentage)
{
    if (percentage < 0.0 or percentage > 1.0) {
        THROW(ValueError() << errmsg{"percentage out of range"});
    }
    const size_t siz = wave.size();
    if (siz == 0) {
        THROW(ValueError() << errmsg{"empty waveform"});
    }
    if (siz == 1) {
        return wave[0];
    }
    size_t mid = percentage * siz;
    mid = std::min(mid, siz - 1);
    std::nth_element(wave.begin(), wave.begin() + mid, wave.end());
    return wave.at(mid);
}

Waveform::real_t WireCell::Waveform::percentile_binned(Waveform::realseq_t& wave, real_t percentage)
{
    const auto mm = std::minmax_element(wave.begin(), wave.end());
    const auto vmin = *mm.first;
    const auto vmax = *mm.second;
    const int nbins = wave.size();
    const auto binsize = (vmax - vmin) / nbins;
    Waveform::realseq_t hist(nbins);
    for (auto val : wave) {
        int bin = int(round((val - vmin) / binsize));
        bin = std::max(0, bin);
        bin = std::min(nbins - 1, bin);
        hist[bin] += 1.0;
    }

    const int imed = wave.size() * percentage;
    int count = 0;
    for (int ind = 0; ind < nbins; ++ind) {
        count += hist[ind];
        if (count > imed) {
            float ret = vmin + ind * binsize;
            return ret;
        }
    }
    // can't reach here, return bogus value.
    return vmin + (vmax - vmin) * percentage;
}

std::pair<int, int> WireCell::Waveform::edge(const realseq_t& wave)
{
    const int size = wave.size();
    int imin = size, imax = size;

    for (int ind = 0; ind < size; ++ind) {
        const real_t val = wave[ind];
        if (val != 0.0) {
            if (imin == size) {  // found start edge
                imin = ind;
            }
            if (imin < size) {
                imax = ind + 1;
            }
        }
    }
    return std::make_pair(imin, imax);
}

thread_local static Eigen::FFT<Waveform::real_t> gEigenFFT;

Waveform::compseq_t WireCell::Waveform::dft(realseq_t wave)
{
    auto v = Eigen::Map<Eigen::VectorXf>(wave.data(), wave.size());
    Eigen::VectorXcf ret = gEigenFFT.fwd(v);
    return compseq_t(ret.data(), ret.data() + ret.size());
}

Waveform::realseq_t WireCell::Waveform::idft(compseq_t spec)
{
    auto v = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size());
    Eigen::VectorXf ret;
    gEigenFFT.inv(ret, v);
    return realseq_t(ret.data(), ret.data() + ret.size());
}

// Linear convolution, returns in1.size()+in2.size()-1.
Waveform::realseq_t WireCell::Waveform::linear_convolve(Waveform::realseq_t in1, Waveform::realseq_t in2, bool truncate)
{
    size_t n1_orig = in1.size(), n2_orig = in2.size();
    size_t n_out = n1_orig + n2_orig - 1;

    in1.resize(n_out, 0);
    in2.resize(n_out, 0);

    auto v1 = Eigen::Map<Eigen::VectorXf>(in1.data(), in1.size());
    auto v2 = Eigen::Map<Eigen::VectorXf>(in2.data(), in2.size());

    Eigen::FFT<Waveform::real_t> trans;

    Eigen::VectorXcf s1 = trans.fwd(v1);
    Eigen::VectorXcf s2 = trans.fwd(v2);
    Eigen::VectorXcf s12 = (s1.array() * s2.array()).matrix();
    Eigen::VectorXf vret;
    trans.inv(vret, s12);
    realseq_t ret(vret.data(), vret.data() + vret.size());
    if (truncate) {
        ret.resize(n1_orig);
    }
    return ret;
}

// Replace old response in wave with new response.
Waveform::realseq_t WireCell::Waveform::replace_convolve(Waveform::realseq_t wave, Waveform::realseq_t newres,
                                                         Waveform::realseq_t oldres, bool truncate)
{
    size_t sizes[3] = {wave.size(), newres.size(), oldres.size()};
    size_t n_out = sizes[0] + sizes[1] + sizes[2] - *std::min_element(sizes, sizes + 3) - 1;

    wave.resize(n_out, 0);
    newres.resize(n_out, 0);
    oldres.resize(n_out, 0);

    auto v1 = Eigen::Map<Eigen::VectorXf>(wave.data(), wave.size());
    auto v2 = Eigen::Map<Eigen::VectorXf>(newres.data(), newres.size());
    auto v3 = Eigen::Map<Eigen::VectorXf>(oldres.data(), oldres.size());

    Eigen::FFT<Waveform::real_t> trans;

    Eigen::VectorXcf s1 = trans.fwd(v1);
    Eigen::VectorXcf s2 = trans.fwd(v2);
    Eigen::VectorXcf s3 = trans.fwd(v3);

    Eigen::VectorXcf s123 = (s1.array() * s2.array() / s3.array()).matrix();

    Eigen::VectorXf vret;
    trans.inv(vret, s123);
    realseq_t ret(vret.data(), vret.data() + vret.size());
    if (truncate) {
        ret.resize(sizes[0]);
    }
    return ret;
}

WireCell::Waveform::BinRangeList WireCell::Waveform::merge(const WireCell::Waveform::BinRangeList& brl)
{
    WireCell::Waveform::BinRangeList tmp(brl.begin(), brl.end());
    WireCell::Waveform::BinRangeList out;
    sort(tmp.begin(), tmp.end());
    Waveform::BinRange last_br = tmp[0];
    out.push_back(last_br);

    for (size_t ind = 1; ind < tmp.size(); ++ind) {
        Waveform::BinRange this_br = tmp[ind];
        if (out.back().second >= this_br.first) {
            out.back().second = this_br.second;
            continue;
        }
        out.push_back(this_br);
    }
    return out;
}

/// Merge two bin range lists, forming a union from any overlapping ranges
WireCell::Waveform::BinRangeList WireCell::Waveform::merge(const WireCell::Waveform::BinRangeList& br1,
                                                           const WireCell::Waveform::BinRangeList& br2)
{
    WireCell::Waveform::BinRangeList out;
    out.reserve(br1.size() + br2.size());
    out.insert(out.end(), br1.begin(), br1.end());
    out.insert(out.end(), br2.begin(), br2.end());
    return merge(out);
}

/// Return a new mapping which is the union of all same channel masks.
WireCell::Waveform::ChannelMasks WireCell::Waveform::merge(const WireCell::Waveform::ChannelMasks& one,
                                                           const WireCell::Waveform::ChannelMasks& two)
{
    WireCell::Waveform::ChannelMasks out = one;
    for (auto const& it : two) {
        int ch = it.first;
        out[ch] = merge(out[ch], it.second);
    }
    return out;
}

void WireCell::Waveform::merge(ChannelMaskMap& one, ChannelMaskMap& two, std::map<std::string, std::string>& name_map)
{
    // loop over second map
    for (auto const& it : two) {
        std::string name = it.first;
        std::string mapped_name;
        if (name_map.find(name) != name_map.end()) {
            mapped_name = name_map[name];
        }
        else {
            mapped_name = name;
        }
        if (one.find(mapped_name) != one.end()) {
            one[mapped_name] = merge(one[mapped_name], it.second);
        }
        else {
            one[mapped_name] = it.second;
        }
    }
}

short WireCell::Waveform::most_frequent(const std::vector<short>& vals)
{
    const size_t nbins = 1 << 16;
    std::vector<unsigned int> hist(nbins, 0);
    for (unsigned short val : vals) {
        hist[val] += 1;
    }
    auto it = std::max_element(hist.begin(), hist.end());
    return it - hist.begin();  // casts back to signed short
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
