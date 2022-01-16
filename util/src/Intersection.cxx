#include "WireCellUtil/Intersection.h"

#include <set>
using namespace std;

using namespace WireCell;

int WireCell::hit_square(int axis0, const Ray& bounds, const Point& point, const Vector& dir, Ray& hits)
{
    const Point& bmin = bounds.first;
    const Point& bmax = bounds.second;

    double hit1[3] = {0}, hit2[3] = {0};

    int hitmask = 0;
    if (0 == dir[axis0]) {
        return hitmask;
    }

    int axis1 = (axis0 + 1) % 3;
    int axis2 = (axis1 + 1) % 3;

    {  // toward the min intercept
        double intercept = bmin[axis0];
        double scale = (intercept - point[axis0]) / dir[axis0];

        double one = point[axis1] + scale * dir[axis1];
        double two = point[axis2] + scale * dir[axis2];

        if (bmin[axis1] <= one && one <= bmax[axis1] && bmin[axis2] <= two && two <= bmax[axis2]) {
            hitmask |= 1;
            hit1[axis0] = intercept;
            hit1[axis1] = one;
            hit1[axis2] = two;
        }
    }

    {  // toward the max intercept
        double intercept = bmax[axis0];
        double scale = (intercept - point[axis0]) / dir[axis0];

        double one = point[axis1] + scale * dir[axis1];
        double two = point[axis2] + scale * dir[axis2];

        if (bmin[axis1] <= one && one <= bmax[axis1] && bmin[axis2] <= two && two <= bmax[axis2]) {
            hitmask |= 2;
            hit2[axis0] = intercept;
            hit2[axis1] = one;
            hit2[axis2] = two;
        }
    }

    hits = Ray(Point(hit1), Point(hit2));
    return hitmask;
}

int WireCell::box_intersection(const Ray& bounds, const Ray& ray, Ray& hits)
{
    PointSet results;
    const Point& point = ray.first;
    const Point dir = (ray.second - ray.first).norm();

    // check each projection
    for (int axis = 0; axis < 3; ++axis) {
        Ray res;
        int got = hit_square(axis, bounds, point, dir, res);

        if (got & 1) {
            // pair<PointSet::iterator, bool> what =
            results.insert(res.first);
        }
        if (got & 2) {
            // pair<PointSet::iterator, bool> what =
            results.insert(res.second);
        }
    }

    if (results.size() > 2) {
        return -1;
    }

    int hitmask = 0;
    for (auto hitit = results.begin(); hitit != results.end(); ++hitit) {
        const Point& hit = *hitit;
        Vector hitdir = hit - point;
        double dot = hitdir.norm().dot(dir);

        if (dot > 0) {  // actually should be closer to +/-1 w/in tolerance
            hits.first = hit;
            hitmask |= 1;
        }
        else {
            hits.second = hit;
            hitmask |= 2;
        }
    }

    return hitmask;
}
