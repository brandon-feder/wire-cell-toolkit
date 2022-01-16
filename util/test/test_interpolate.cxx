#include "WireCellUtil/Interpolate.h"
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>

#include <iomanip>
#include <iostream>

// https://www.boost.org/doc/libs/1_75_0/libs/math/doc/html/math_toolkit/cardinal_cubic_b.html

using boost::math::interpolators::cardinal_cubic_b_spline;

using namespace WireCell;

int main()
{
    std::vector<double> f{0.01, -0.02, 0.3, 0.8, 1.9, -8.78, -22.6};
    const double xstep = 0.01;
    const double x0 = xstep;
    cardinal_cubic_b_spline<double> spline(f.begin(), f.end(), x0, xstep);

    linterp<double> lin(f.begin(), f.end(), x0, xstep);

    for (double x = 0; x < x0 + xstep * 10; x += 0.1 * xstep) {
        std::cout << std::setprecision(3) << std::fixed << "x=" << x << "\tlin(x)=" << lin(x)
                  << "\tspline(x)=" << spline(x) << "\n";
    }
    return 0;
}
