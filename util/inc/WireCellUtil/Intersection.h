#ifndef WIRECELLUTIL_INTERSECTION
#define WIRECELLUTIL_INTERSECTION

#include "WireCellUtil/Point.h"

namespace WireCell {

    /** Determine a 2D square is intersected by a 3D ray projected to
     * its plane.
     *
     * \param axis0 is the axis number (0,1,2) to which the projected
     * plane is perpendicular.
     *
     * \param bounds is a raw from opposite corners of the bounding box.
     *
     * \param point is a WireCell::Point from which a ray emanates.
     *
     * \param dir is a WireCell::Vector in the direction of the ray.
     *
     * \param hits is the returned intersection(s).
     *
     * \return a "hit mask" with 0 indicating the ray did not
     * intersect, 1 or 2 meaning the first or second point in the ray
     * is valid and 3 meaning both are.
     */

    int hit_square(int axis0, const Ray& bounds, const Point& point, const Vector& dir, Ray& hits);

    /** Determine if a ray hits a rectangular box aligned with the
     * Cartesian axes.
     *
     * See WireCelll::hit_square for definition of arguments and return value.
     */
    int box_intersection(const Ray& bounds, const Ray& ray, Ray& hits);

}  // namespace WireCell

#endif
