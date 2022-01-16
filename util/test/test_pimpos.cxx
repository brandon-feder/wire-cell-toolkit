#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace WireCell;

template <typename Type>
std::string str(const std::pair<Type, Type>& tt)
{
    stringstream ss;
    ss << "[" << tt.first << "," << tt.second << "]";
    return ss.str();
}

int main()
{
    const double pitch_dist = 3 * units::mm;
    const int nwires = 2001;
    const double halfwireextent = pitch_dist * 0.5 * (nwires - 1);
    cerr << "Wires at extremes +/- " << halfwireextent / units::mm << "mm\n";
    Pimpos pimpos(nwires, -halfwireextent, halfwireextent);

    const Point zero(0, 0, 0);
    const Point pt1(1 * units::m, 2 * units::m, 3 * units::m);
    const Point pt2(1 * units::mm, 2 * units::mm, 3 * units::mm);

    for (int ind = 0; ind < 3; ++ind) {
        cerr << "axis" << ind << ": " << pimpos.axis(ind) << endl;
    }

    {
        auto val = pimpos.relative(zero);
        cerr << "relative: " << val << endl;
        Assert(val == -1.0 * pimpos.origin());
    }

    {
        auto val = pimpos.distance(zero, 0);
        cerr << "distance 0: " << val << endl;
        Assert(val == 0.0);
    }

    auto rb = pimpos.region_binning();
    cerr << str(rb.range()) << " " << str(rb.irange()) << " binsize:" << rb.binsize() / units::mm << "mm\n";
    Assert(rb.nbins() == nwires);
    Assert(rb.min() == -3001.5 * units::mm);
    Assert(rb.max() == 3001.5 * units::mm);
    Assert(rb.binsize() == pitch_dist);

    Assert(rb.bin(0) == nwires / 2);
    Assert(rb.center(rb.bin(0)) == 0.0);

    Assert(rb.inside(0.0));
    const double outside = halfwireextent + pitch_dist;
    Assert(!rb.inside(outside));

    // auto ib = pimpos.impact_binning();

    const int center_wire = nwires / 2;
    auto center_wi = pimpos.closest(0.0);
    Assert(center_wi.first == center_wire);
    Assert(center_wi.second == 0);

    const int center_imp = pimpos.wire_impact(center_wire);
    auto ref1 = pimpos.reflect(center_wire, center_imp + 4);
    Assert(ref1 == center_imp - 4);

    return 0;
}
