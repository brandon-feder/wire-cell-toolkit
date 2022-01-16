#include "WireCellUtil/Point.h"
#include "WireCellUtil/BoundingBox.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Units.h"

#include <iomanip>
#include <iostream>
using namespace std;
using namespace WireCell;

int main()
{
    {
        Point pp1(1, -500, -495);
        Point pp2(1, 500, -495);
        PointSet results;
        results.insert(pp1);
        results.insert(pp2);
        AssertMsg(2 == results.size(), "failed to insert");
    }

    {
        Point origin(0, 0, 0);
        Vector zdir(0, 0, 1);
        Point pt(0 * units::mm, 3.92772 * units::mm, 5.34001 * units::mm);
        double dot = zdir.dot(pt - origin);
        cerr << "origin=" << origin / units::mm << ", zdir=" << zdir << ", pt=" << pt / units::mm
             << " dot=" << dot / units::mm << endl;
    }

    Point p1(1, 2, 3);
    Assert(p1.x() == 1 && p1.y() == 2 && p1.z() == 3);

    Point p2 = p1;

    Assert(p1 == p2);

    Point p3(p1);
    Assert(p1 == p3);

    PointF pf(p1);
    Assert(Point(pf) == p1);
    Assert(pf == PointF(p1));

    Point ps = p1 + p2;
    Assert(ps.x() == 2);

    Assert(p1.norm().magnitude() == 1.0);

    double eps = (1 - 1e-11);
    Point peps = p1 * eps;
    cerr << "Epsilon=" << std::setprecision(12) << eps << " peps=" << peps << endl;
    PointSet pset;
    pset.insert(p1);
    pset.insert(p2);
    pset.insert(p3);
    pset.insert(ps);
    pset.insert(peps);
    for (auto pit = pset.begin(); pit != pset.end(); ++pit) {
        cerr << *pit << endl;
    }
    AssertMsg(pset.size() == 2, "tolerance set broken");

    Point pdiff = p1;
    pdiff.set(3, 2, 1);
    AssertMsg(p1 != pdiff, "Copy on write failed.");

    Point foo;
    /// temporarily make this a really big loop to test no memory leakage
    for (int ind = 0; ind < 1000; ++ind) {
        foo = p1;
        foo = p2;
        foo = p3;
    }

    const Ray r1(Point(3.75, -323.316, -500), Point(3.75, 254.034, 500));
    const Ray r2(Point(2.5, -254.034, 500), Point(2.5, 323.316, -500));
    const Ray c12 = ray_pitch(r1, r2);
    cerr << "r1=" << r1 << "\n"
         << "r2=" << r2 << "\n"
         << "rp=" << c12 << "\n";

    BoundingBox bb(pset.begin(), pset.end());
    bb.inside(Point());

    {
        Point p(1, 2, 3);
        Assert(p);
        cerr << "  valid: " << p << endl;
        p.invalidate();
        Assert(!p);
        // cerr << "invalid: " << p << endl;
        p.set();
        Assert(p);
        cerr << "revalid: " << p << endl;
    }
}
