#ifndef WIRECELL_BUFFEREDHISTOGRAM2D
#define WIRECELL_BUFFEREDHISTOGRAM2D

#include <vector>
#include <deque>

namespace WireCell {

    /** A 2D histogram which can expand its range based on filling and
     * have its edge sliced off.
     */
    class BufferedHistogram2D {
        double m_xmin, m_ymin, m_xbinsize, m_ybinsize;

        typedef std::vector<double> XBin;
        typedef std::deque<XBin> XBinDeque;
        XBinDeque m_xbindeque;

       public:
        /// Create a buffered 2d histogram with lower corner at xmin,
        /// ymin and with bins of given sizes.
        BufferedHistogram2D(double xbinsize = 1.0, double ybinsize = 1.0, double xmin = 0.0, double ymin = 0.0);

        /// Add v to the bin holding x,y.  Return false if point is out of bounds.
        bool fill(double x, double y, double v = 1.0);

        /// Slice off the lowest X bin and return it as a vector.
        /// Note, the zeroth element of the vector corresponds to the
        /// bin starting at ymin but the length of vector may not
        /// represent the global ymax.
        std::vector<double> popx();

        /// popy is not yet implemented.
        // std::vector<double> popy();

        // return buffer depth
        int size() const { return m_xbindeque.size(); }
        int xsize() const { return size(); }
        // return the maximum size in Y
        int ysize() const;

        double xmin() const { return m_xmin; }
        double ymin() const { return m_ymin; }
        double xmax() const;
        double ymax() const;
        double xbinsize() const { return m_xbinsize; }
        double ybinsize() const { return m_ybinsize; }
    };

}  // namespace WireCell

#endif
