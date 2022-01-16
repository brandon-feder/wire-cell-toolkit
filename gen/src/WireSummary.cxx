#include "WireCellGen/WireSummary.h"
#include "WireCellIface/IWireSelectors.h"

#include <iterator>
#include <map>

using namespace WireCell;
using namespace std;

// Return a Ray going from the center point of wires[0] to a point on
// wire[1] and perpendicular to both.
static Ray pitch2(const IWire::vector& wires)
{
    // Use two consecutive wires from the center to determine the pitch.
    int ind = wires.size() / 2;
    IWire::pointer one = wires[ind];
    IWire::pointer two = wires[ind + 1];
    const Ray p = ray_pitch(one->ray(), two->ray());

    // Move the pitch to the first wire
    IWire::pointer zero = wires[0];
    const Vector center0 = zero->center();
    return Ray(center0, center0 + ray_vector(p));
}

struct WirePlaneCache {
    WirePlaneId wpid;
    IWire::vector wires;
    Ray pitch_ray;
    Vector pitch_vector;
    Vector pitch_unit;
    double pitch_mag;

    WirePlaneCache(WirePlaneId wpid, const IWire::vector& all_wires)
      : wpid(wpid)  // maybe one day support more then one face/apa
    {
        copy_if(all_wires.begin(), all_wires.end(), back_inserter(wires), select_uvw_wires[wpid.index()]);
        pitch_ray = pitch2(wires);
        pitch_vector = ray_vector(pitch_ray);  // cache
        pitch_unit = pitch_vector.norm();      // the
        pitch_mag = pitch_vector.magnitude();  // things!
    }

    IWire::pointer wire_by_index(int index)
    {
        if (index < 0 || index >= (int) wires.size()) {
            return 0;
        }
        return wires[index];
    }

    double pitch_distance(const Point& point) { return ray_dist(pitch_ray, point); }
};

struct WireSummary::WireSummaryCache {
    IWire::vector wires;
    BoundingBox bb;
    WirePlaneCache* plane_cache[3];

    std::map<int, IWireSegmentSet> chan2wire;

    WireSummaryCache(const IWire::vector& wires)
      : wires(wires)
    {
        for (auto wire : wires) {
            bb(wire->ray());
            chan2wire[wire->channel()].insert(wire);
        }

        plane_cache[0] = new WirePlaneCache(WirePlaneId(kUlayer), wires);
        plane_cache[1] = new WirePlaneCache(WirePlaneId(kVlayer), wires);
        plane_cache[2] = new WirePlaneCache(WirePlaneId(kWlayer), wires);
    }

    ~WireSummaryCache()
    {
        for (int ind = 0; ind < 3; ++ind) {
            delete plane_cache[ind];
        }
    }

    WirePlaneCache* plane(WirePlaneId wpid) { return plane(wpid.index()); }
    WirePlaneCache* plane(int index)
    {
        if (index < 0 || index > 2) return 0;
        return plane_cache[index];
    }

    IWire::vector by_chan(int chan)
    {
        IWireSegmentSet& got = chan2wire[chan];
        return IWire::vector(got.begin(), got.end());
    }
};

static IWire::vector dummy_vector;

const BoundingBox& WireSummary::box() const
{
    if (!m_cache) {
        static BoundingBox bbdummy;
        return bbdummy;
    }
    return m_cache->bb;
}

IWire::pointer WireSummary::closest(const Point& point, WirePlaneId wpid) const
{
    if (!m_cache) {
        return 0;
    }
    WirePlaneCache* wpc = m_cache->plane(wpid);
    if (!wpc) {
        return 0;
    }

    double dist = wpc->pitch_distance(point);
    return wpc->wire_by_index(dist / wpc->pitch_mag);
}

IWirePair WireSummary::bounding_wires(const Point& point, WirePlaneId wpid) const
{
    if (!m_cache) {
        return IWirePair();
    }
    WirePlaneCache* wpc = m_cache->plane(wpid);
    if (!wpc) {
        return IWirePair();
    }

    IWire::pointer wire = closest(point, wpid);
    if (!wire) return IWirePair();

    int index = wire->index();

    Vector topoint = point - wire->ray().first;
    double dot = wpc->pitch_unit.dot(topoint);
    int other_index = index - 1;
    if (dot > 0) {
        other_index = index + 1;
    }

    IWire::pointer other_wire = wpc->wire_by_index(other_index);

    if (index < other_index) {
        return IWirePair(IWire::pointer(wire), IWire::pointer(other_wire));
    }
    return IWirePair(IWire::pointer(other_wire), IWire::pointer(wire));
}

double WireSummary::pitch_distance(const Point& point, WirePlaneId wpid) const
{
    if (!m_cache) {
        return 0;
    }
    WirePlaneCache* wpc = m_cache->plane(wpid);
    if (!wpc) {
        return 0;
    }
    return wpc->pitch_distance(point);
}

const Vector& WireSummary::pitch_direction(WirePlaneId wpid) const
{
    static Vector dummy;
    if (!m_cache) {
        return dummy;
    }
    WirePlaneCache* wpc = m_cache->plane(wpid);
    if (!wpc) {
        return dummy;
    }
    return wpc->pitch_unit;
}

IWire::vector WireSummary::by_channel(int channel) const
{
    if (!m_cache) {
        return IWire::vector();
    }
    return m_cache->by_chan(channel);
}

WireSummary::WireSummary(const IWire::vector& wires)
  : m_cache(0)
{
    m_cache = new WireSummaryCache(wires);
}
WireSummary::~WireSummary() {}
