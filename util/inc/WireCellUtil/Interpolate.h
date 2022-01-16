/**
   Interpolationn helpers.

   See test_interpolate.cxx.
 */

#ifndef WIRECELLUTIL_INTERPOLATE
#define WIRECELLUTIL_INTERPOLATE

#include <vector>

namespace WireCell {

    /**
       Use like:

          linterp<double> lin(f.begin(), f.end(), x0, xstep);
          ...
          double y = lin(42.0);

       where "f" is some kind of collection of doubles.

     */
    template <class Real>
    class linterp {
       public:
        template <class BidiIterator>
        linterp(BidiIterator f, BidiIterator end_p, Real left_endpoint, Real step)
          : m_dat(f, end_p)
          , m_le(left_endpoint)
          , m_step(step)
        {
            m_re = m_le + m_step * (m_dat.size() - 1);
        }

        Real operator()(Real x) const
        {
            if (x <= m_le) return m_dat.front();
            if (x >= m_re) return m_dat.back();

            int ind = int((x - m_le) / m_step);
            Real y0 = m_dat[ind];
            Real y1 = m_dat[ind + 1];
            Real x0 = m_le + ind * m_step;

            return y0 + (x - x0) * (y1 - y0) / m_step;
        }

       private:
        std::vector<Real> m_dat;
        Real m_le, m_re, m_step;
    };

    /** You may also want to use Boost for fancier interpolation.
     * They have similar calling interface:
     #include <boost/math/interpolators/cubic_b_spline.hpp>
     #include <iostream>
     ...
     boost::math::cubic_b_spline<double> spline(f.begin(), f.end(), x0, xstep);
     spline(42);
     *
     * More info here:
     https://www.boost.org/doc/libs/1_65_0/libs/math/doc/html/math_toolkit/interpolate/cubic_b.html
    */

}  // namespace WireCell

#endif
