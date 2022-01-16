#ifndef WIRECELL_WAVEFORM
#define WIRECELL_WAVEFORM

#include <map>
#include <cstdint>
#include <vector>
#include <complex>
#include <numeric>
#include <algorithm>
#include <string>

// FIXME: remove the hard-wired Eigen::FFT related in favor of dynamic
// DFT with Aux/DftTools.h.
#include <Eigen/Core>
#include <unsupported/Eigen/FFT>

namespace WireCell {

    namespace Waveform {

        /// The type for the signal in each bin.
        typedef float real_t;

        /// The type for the spectrum in each bin.
        typedef std::complex<float> complex_t;

        /// A sequence is an ordered array of values (real or
        /// complex).  By itself it is not associated with a domain.
        template <typename Val>
        using Sequence = std::vector<Val>;

        // A real-valued sequence, eg for discrete signal values.
        typedef Sequence<real_t> realseq_t;

        /// A complex-valued sequence, eg for discrete spectrum powers.
        typedef Sequence<complex_t> compseq_t;

        /// A half-open range of bins (from first bin to one past last bin)
        typedef std::pair<int, int> BinRange;

        /// A list of bin ranges.
        typedef std::vector<BinRange> BinRangeList;

        /// Return a new list with any overlaps formed into unions.
        BinRangeList merge(const BinRangeList& br);

        /// Merge two bin range lists, forming a union from any overlapping ranges
        BinRangeList merge(const BinRangeList& br1, const BinRangeList& br2);

        /// Map channel number to a vector of BinRanges
        typedef std::map<int, BinRangeList> ChannelMasks;

        /// Return a new mapping which is the union of all same channel masks.
        ChannelMasks merge(const ChannelMasks& one, const ChannelMasks& two);

        /// Collect channel masks by some label.
        typedef std::map<std::string, ChannelMasks> ChannelMaskMap;

        // merge second maskmap into the first maskmap
        void merge(ChannelMaskMap& one, ChannelMaskMap& two, std::map<std::string, std::string>& name_map);

        /// A range of time
        typedef std::pair<double, double> Period;

        /// A range of frequency
        typedef std::pair<double, double> Band;

        /// A domain of a sequence is bounded by the time or frequency
        /// at the start of its first element and that at the end of
        /// its last.
        typedef std::pair<double, double> Domain;
        /// Return the number of samples needed to cover the domain with sample size width.
        inline int sample_count(const Domain& domain, double width) { return (domain.second - domain.first) / width; }
        /// Return the sample size if domain is equally sampled with given number of samples
        inline double sample_width(const Domain& domain, int count) { return (domain.second - domain.first) / count; }

        /// Return the begin/end sample numbers inside the a subdomain of a domain with nsamples total.
        std::pair<int, int> sub_sample(const Domain& domain, int nsamples, const Domain& subdomain);

        /// Return a new sequence resampled and interpolated from the
        /// original wave defined over the domain to a new domain of
        /// nsamples.
        template <typename Val>
        Sequence<Val> resample(const Sequence<Val>& wave, const Domain& domain, int nsamples, const Domain& newdomain)
        {
            const int oldnsamples = wave.size();
            const double oldstep = sample_width(domain, oldnsamples);
            const double step = sample_width(newdomain, nsamples);
            Sequence<Val> ret;
            for (int ind = 0; ind < nsamples; ++ind) {
                double cursor = newdomain.first + ind * step;
                double oldfracsteps = (cursor - domain.first) / oldstep;
                int oldind = int(oldfracsteps);
                if (cursor <= domain.first || oldind <= 0) {
                    ret.push_back(wave[0]);
                    continue;
                }
                if (cursor >= domain.second or oldind + 1 >= oldnsamples) {
                    ret.push_back(wave[oldnsamples - 1]);
                    continue;
                }
                double d1 = oldfracsteps - oldstep * oldind;
                double d2 = oldstep - d1;
                Val newval = (wave[oldind] * d1 + wave[oldind + 1] * d2) / oldstep;
                ret.push_back(newval);
            }
            return ret;
        }

        /// Return the real part of the sequence
        realseq_t real(const compseq_t& seq);
        /// Return the imaginary part of the sequence
        realseq_t imag(const compseq_t& seq);
        /// Return the magnitude or absolute value of the sequence
        realseq_t magnitude(const compseq_t& seq);
        /// Return the phase or arg part of the sequence
        realseq_t phase(const compseq_t& seq);
        /// Uplift a real sequence to a complex one (with zero imaginary parts)
        compseq_t complex(const realseq_t& real);
        /// Pack individual real/imag parts into complex sequence
        compseq_t complex(const realseq_t& real, const realseq_t& imag);

        /// Increase (shift) sequence values by scalar
        template <typename Val>
        void increase(Sequence<Val>& seq, Val scalar)
        {
            std::transform(seq.begin(), seq.end(), seq.begin(), [scalar](Val x) { return x + scalar; });
        }
        inline void increase(Sequence<float>& seq, double scalar) { increase(seq, (float) scalar); }

        /// Increase (shift) sequence values by values in another sequence
        template <typename Val>
        void increase(Sequence<Val>& seq, const Sequence<Val>& other)
        {
            std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(), std::plus<Val>());
        }

        /// Scale (multiply) sequence values by scalar
        template <typename Val>
        void scale(Sequence<Val>& seq, Val scalar)
        {
            std::transform(seq.begin(), seq.end(), seq.begin(), [scalar](Val x) { return x * scalar; });
        }
        inline void scale(Sequence<float>& seq, double scalar) { scale(seq, (float) scalar); }

        /// Scale (multiply) seq values by values from the other sequence.
        template <typename Val>
        void scale(Sequence<Val>& seq, const Sequence<Val>& other)
        {
            std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(), std::multiplies<Val>());
        }
        /// Shrink (divide) seq values by values from the other sequence.
        template <typename Val>
        void shrink(Sequence<Val>& seq, const Sequence<Val>& other)
        {
            std::transform(seq.begin(), seq.end(), other.begin(), seq.begin(), std::divides<Val>());
        }

        /// Return a pair of indices into wave which bound non-zero
        /// region.  First index is of first non-zero sample, second
        /// index is one past last non-zero sample.  If entire wave is
        /// empty then both are set to size().
        std::pair<int, int> edge(const realseq_t& wave);

        /// Return sum of all entries in sequence.
        template <typename Val>
        Val sum(const Sequence<Val>& seq)
        {
            return std::accumulate(seq.begin(), seq.end(), Val());
        }

        /// Return sum of square of all entries in sequence.
        template <typename Val>
        Val sum2(const Sequence<Val>& seq)
        {
            return std::accumulate(seq.begin(), seq.end(), Val(),
                                   [](const Val& bucket, Val x) { return bucket + x * x; });
        }

        // Return the mean and (population) RMS over a waveform signal.
        std::pair<double, double> mean_rms(const realseq_t& wave);

        // Return the median value.  This is rather slow as it
        // involves a sort.
        real_t median(realseq_t& wave);

        // Return the median value.  This is faster but introduces
        // inaccuracies due to binning.
        real_t median_binned(realseq_t& wave);

        real_t percentile(realseq_t& wave, real_t percentage);
        real_t percentile_binned(realseq_t& wave, real_t percentage);

        /// Discrete Fourier transform of real sequence.  Returns full
        /// spectrum.  No normalization scaling applied
        compseq_t dft(realseq_t seq);

        // Linear convolution, returns in1.size()+in2.size()-1.  If
        // truncate is false then the returned sequence will be
        // truncated to length that of the first input.  Otherwise the
        // function is symmetric between the two inputs.
        realseq_t linear_convolve(Waveform::realseq_t in1, Waveform::realseq_t in2, bool truncate = true);

        // Replace old response in wave with new response.  If
        // truncate is false then the returned sequence will be the
        // length required for linear convolution.  This is the sum of
        // the sizes of all input less one and less the smallest.
        realseq_t replace_convolve(Waveform::realseq_t wave, Waveform::realseq_t newres, Waveform::realseq_t oldres,
                                   bool truncate = true);

        /// Inverse, discrete Fourier transform.  Expects full
        /// spectrum (twice Nyquist frequency).  Applies the
        /// 1/Nsamples normalization.
        realseq_t idft(compseq_t spec);

        /// Return the smallest, most frequent value to appear in vector.
        short most_frequent(const std::vector<short>& vals);

        class FFT {
           public:
            FFT() {}
            inline compseq_t dft(realseq_t wave)
            {
                auto v = Eigen::Map<Eigen::VectorXf>(wave.data(), wave.size());
                Eigen::VectorXcf ret = trans.fwd(v);
                return compseq_t(ret.data(), ret.data() + ret.size());
            }
            inline realseq_t idft(compseq_t spec)
            {
                auto v = Eigen::Map<Eigen::VectorXcf>(spec.data(), spec.size());
                Eigen::VectorXf ret;
                trans.inv(ret, v);
                return realseq_t(ret.data(), ret.data() + ret.size());
            }

           private:
            Eigen::FFT<Waveform::real_t> trans;
        };

    }  // namespace Waveform
}  // namespace WireCell

#endif
