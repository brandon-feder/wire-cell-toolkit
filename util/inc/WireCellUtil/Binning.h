#ifndef WIRECELLUTIL_BINNING_H
#define WIRECELLUTIL_BINNING_H

#include <map>  // for std::pair
#include <cmath>
#include <iostream>  // for ostream

namespace WireCell {

    /** A binning is a uniform discretization of a linear space.

        This class largely provides methods that give semantic labels
        to various calculations related to a binned region.
    */
    class Binning {
        int m_nbins;
        double m_minval, m_maxval, m_binsize;

       public:
        /** Create a binning
            \param nbins gives the number of uniform, discrete separation between bounds.
            \param minval gives the lower bound of the linear space (low edge of bin 0)
            \param maxval gives the upper bound of the linear space (high edge of bin nbins-1)
        */
        Binning(int nbins, double minval, double maxval)
          : m_nbins(0)
          , m_minval(0)
          , m_maxval(0)
          , m_binsize(0)
        {
            set(nbins, minval, maxval);
        }
        Binning()
          : m_nbins(0)
          , m_minval(0)
          , m_maxval(0)
          , m_binsize(0)
        {
        }

        // Post constructor setting
        void set(int nbins, double minval, double maxval)
        {
            m_nbins = nbins;
            m_minval = minval;
            m_maxval = maxval;
            m_binsize = ((maxval - minval) / nbins);
        }

        // Access given number of bins.
        int nbins() const { return m_nbins; }

        // Access given minimum range of binning.
        double min() const { return m_minval; }

        // Access given maximum range of binning.
        double max() const { return m_maxval; }

        /// Return the max-min
        double span() const { return m_maxval - m_minval; }

        // Binning as a range.
        std::pair<double, double> range() const { return std::make_pair(m_minval, m_maxval); }

        // Return half open range of bin indices or alternatively
        // fully closed range of edge indices.
        std::pair<int, int> irange() const { return std::make_pair(0, m_nbins); }

        // Access resulting bin size..
        double binsize() const { return m_binsize; }

        /// Return the bin containing value.  If val is in range,
        /// return value is [0,nbins-1] but no range checking is
        /// performed.
        int bin(double val) const { return int((val - m_minval) / m_binsize); }

        /// Return the center value of given bin.  Range checking is
        /// not done.
        double center(int ind) const { return m_minval + (ind + 0.5) * m_binsize; }

        /// Return the edge, nominally in [0,nbins] closest to the
        /// given value.  Range checking is not done so returned edge
        /// may be outside of range.
        int edge_index(double val) const { return int(round((val - m_minval) / m_binsize)); }

        /// Return the position of the given bin edge.  Range checking
        /// is not done.
        double edge(int ind) const { return m_minval + ind * m_binsize; }

        /// Return true value is in range.  Range is considered half
        /// open.  Ig, edge(nbins) is not inside range.
        bool inside(double val) const { return m_minval <= val && val < m_maxval; }

        /// Return true if bin is in bounds.
        bool inbounds(int bin) const { return 0 <= bin && bin < m_nbins; }

        /// Return half-open bin range which covers the range of
        /// values.  Bounds are forced to return values in [0,nbins].
        std::pair<int, int> sample_bin_range(double minval, double maxval) const
        {
            return std::make_pair(std::max(bin(minval), 0), std::min(bin(maxval) + 1, m_nbins));
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const WireCell::Binning& bins)
    {
        os << bins.nbins() << "@[" << bins.min() << "," << bins.max() << "]";
        return os;
    }

}  // namespace WireCell

#endif /* WIRECELLUTIL_BINNING_H */
