#include "WireCellUtil/RayTiling.h"
#include "WireCellUtil/Logging.h"

#include <algorithm>
#include <sstream>

using namespace WireCell;
using namespace WireCell::RayGrid;

Activity::Activity(layer_index_t layer)
  : m_span{}
  , m_layer(layer)
  , m_offset(0)
  , m_threshold(0)
{
}

Activity::Activity(layer_index_t layer, size_t span, double value, int offset, double threshold)
  : m_span(span, value)
  , m_layer(layer)
  , m_offset(offset)
  , m_threshold(threshold)
{
}

Activity::Activity(layer_index_t layer, const range_t& span, int offset, double threshold)
  : m_span{}
  , m_layer(layer)
  , m_offset(offset)
  , m_threshold(threshold)
{
    if (span.first == span.second) {
        return;
    }
    iterator_t b = span.first;
    while (b != span.second and *b <= m_threshold) {
        ++b;
        ++m_offset;
    }
    iterator_t e = span.second;
    while (e > b and *(e - 1) <= m_threshold) {
        --e;
    }
    m_span.insert(m_span.begin(), b, e);
}

Activity::iterator_t Activity::begin() const { return m_span.begin(); }

Activity::iterator_t Activity::end() const { return m_span.end(); }

bool Activity::empty() const { return m_span.empty(); }

int Activity::pitch_index(const iterator_t& it) const { return m_offset + it - m_span.begin(); }
// Produce a subspan activity between pitch indices [pi1, pi2)
Activity Activity::subspan(int abs_beg, int abs_end) const
{
    const int rel_beg = abs_beg - m_offset;
    const int rel_end = abs_end - m_offset;

    if (rel_beg < 0 or rel_beg >= rel_end or rel_end > (int) m_span.size()) {
        spdlog::debug("activity::subspan bogus absolute:[{},{}] m_offset={} span.size={}", abs_beg, abs_end, m_offset,
                      m_span.size());
        return Activity(m_layer);
    }

    return Activity(m_layer, {begin() + rel_beg, begin() + rel_end}, abs_beg);
}

Strip Activity::make_strip(const Activity::range_t& r) const
{
    return Strip{m_layer, std::make_pair(pitch_index(r.first), pitch_index(r.second))};
}

strips_t Activity::make_strips() const
{
    strips_t ret;
    for (const auto& ar : active_ranges()) {
        ret.push_back(make_strip(ar));
    }
    return ret;
}

Activity::ranges_t Activity::active_ranges() const
{
    ranges_t ret;
    range_t current{end(), end()};

    for (auto it = begin(); it != end(); ++it) {
        // entering active range
        if (current.first == end() and *it > m_threshold) {
            current.first = it;
            continue;
        }
        // exiting active range
        if (current.first != end() and *it <= 0.0) {
            current.second = it;
            ret.push_back(current);
            current.first = end();
        }
    }
    if (current.first != end()) {
        current.second = end();
        ret.push_back(current);
    }
    return ret;
}

/*********************************/

static crossings_t find_corners(const Strip& one, const Strip& two)
{
    crossings_t ret;

    const auto a = one.addresses(), b = two.addresses();

    ret.push_back(std::make_pair(a.first, b.first));
    ret.push_back(std::make_pair(a.first, b.second));
    ret.push_back(std::make_pair(a.second, b.first));
    ret.push_back(std::make_pair(a.second, b.second));
    return ret;
}

void Blob::add(const Coordinates& coords, const Strip& strip)
{
    const size_t nstrips = m_strips.size();

    if (nstrips == 0) {  // special case
        m_strips.push_back(strip);
        return;
    }

    if (nstrips == 1) {  // special case
        m_strips.push_back(strip);
        m_corners = find_corners(m_strips.front(), m_strips.back());
        return;
    }

    crossings_t surviving;

    // See what old corners are inside the new strip
    for (const auto& c : m_corners) {
        const double pitch = coords.pitch_location(c.first, c.second, strip.layer);
        const int pind = coords.pitch_index(pitch, strip.layer);

        if (strip.in(pind)) {
            surviving.push_back(c);
        }
    }

    // see what new corners are inside all old strips;
    for (size_t si1 = 0; si1 < nstrips; ++si1) {
        auto corners = find_corners(m_strips[si1], strip);
        for (const auto& c : corners) {
            // check each corner if inside all other strips
            bool miss = false;
            for (size_t si2 = 0; si2 < nstrips; ++si2) {
                if (si1 == si2) {
                    continue;
                }
                const auto& s2 = m_strips[si2];
                double pitch = coords.pitch_location(c.first, c.second, s2.layer);
                const int pind = coords.pitch_index(pitch, s2.layer);
                if (s2.in(pind)) {
                    continue;
                }

                miss = true;
                break;
            }
            if (!miss) {
                surviving.push_back(c);
            }
        }
    }
    m_corners = surviving;
    m_strips.push_back(strip);
}

const crossings_t& Blob::corners() const { return m_corners; }

Tiling::Tiling(const Coordinates& coords)
  : m_coords(coords)
{
}

blobs_t Tiling::operator()(const Activity& activity)
{
    auto strips = activity.make_strips();
    const size_t nstrips = strips.size();
    blobs_t ret(nstrips);
    for (size_t ind = 0; ind < nstrips; ++ind) {
        ret[ind].add(m_coords, strips[ind]);
    }
    return ret;
}

Activity Tiling::projection(const Blob& blob, const Activity& activity)
{
    // special case.  A single layer blob effectively extends to
    // infinity along the ray direction so any possible activity
    // projects.
    if (blob.strips().size() == 1) {
        return activity;
    }

    const double pitch_mag = m_coords.pitch_mags()[activity.layer()];

    // find extreme pitches
    std::vector<double> pitches;
    const auto corners = blob.corners();
    if (corners.empty()) {
        return Activity(activity.layer());
    }
    for (const auto& c : blob.corners()) {
        const double p = m_coords.pitch_location(c.first, c.second, activity.layer());
        pitches.push_back(p);
    }
    if (pitches.empty()) {
        return Activity(activity.layer());
    }

    auto pbeg = pitches.begin();
    auto pend = pitches.end();

    const auto mm = std::minmax_element(pbeg, pend);
    int pind1 = std::floor((*mm.first) / pitch_mag);
    int pind2 = std::ceil((*mm.second) / pitch_mag);

    const int apind1 = activity.pitch_index(activity.begin());
    const int apind2 = activity.pitch_index(activity.end());

    if (pind2 <= apind1 or pind1 >= apind2) {
        return Activity(activity.layer());
    }

    pind1 = std::max(pind1, activity.pitch_index(activity.begin()));
    pind2 = std::min(pind2, activity.pitch_index(activity.end()));

    Activity ret = activity.subspan(pind1, pind2);
    return ret;
}

std::string Blob::as_string() const
{
    std::stringstream ss;
    ss << *this << "\n";
    const auto& strips = this->strips();
    ss << "\tstrips (" << strips.size() << "):\n";
    for (const auto& s : strips) {
        ss << "\t\t" << s << "\n";
    }
    const auto corners = this->corners();
    ss << "\tcorners (" << corners.size() << "):\n";
    for (const auto& c : corners) {
        ss << "\t\t" << c << "\n";
    }
    return ss.str();
}

std::string Activity::as_string() const
{
    std::stringstream ss;
    ss << *this << "\n";
    for (auto strip : make_strips()) {
        ss << "\t" << strip << "\n";
    }
    return ss.str();
}

blobs_t Tiling::operator()(const blobs_t& prior_blobs, const Activity& activity)
{
    blobs_t ret;

    for (const auto& blob : prior_blobs) {
        Activity proj = projection(blob, activity);
        if (proj.empty()) {
            continue;
        }
        auto strips = proj.make_strips();
        for (auto strip : strips) {
            Blob newblob = blob;  // copy
            newblob.add(m_coords, strip);
            if (newblob.corners().empty()) {
                continue;
            }
            ret.push_back(newblob);
        }
    }

    return ret;
}

size_t WireCell::RayGrid::drop_invalid(blobs_t& blobs)
{
    const auto end = std::partition(blobs.begin(), blobs.end(), [](const Blob& b) { return b.valid(); });
    const size_t dropped = blobs.end() - end;
    blobs.resize(end - blobs.begin());
    return dropped;
}

void WireCell::RayGrid::prune(const Coordinates& coords, blobs_t& blobs)
{
    for (auto& blob : blobs) {
        auto& strips = blob.strips();
        const int nlayers = strips.size();
        std::vector<std::vector<grid_index_t> > mms(nlayers);
        for (const auto& corner : blob.corners()) {
            // fixme off by one bugs here?  Adding the two rays making
            // up a corner adds a pitch-bin-edge.  Adding the ray
            // crossing point measured in the 3rd layer pitch adds a
            // bin pitch-bin-content which should be either floor()'ed
            // or ceil()'ed (or both?)

            mms[corner.first.layer].push_back(corner.first.grid);
            mms[corner.second.layer].push_back(corner.second.grid);

            // Check every layer not forming the corner
            for (int layer = 0; layer < nlayers; ++layer) {
                if (corner.first.layer == layer or corner.second.layer == layer) {
                    continue;
                }
                const double ploc = coords.pitch_location(corner.first, corner.second, layer);
                const int pind = coords.pitch_index(ploc, layer);
                mms[layer].push_back(pind);
                mms[layer].push_back(pind + 1);
            }
        }

        for (int layer = 0; layer < nlayers; ++layer) {
            auto mm = std::minmax_element(mms[layer].begin(), mms[layer].end());
            strips[layer].bounds.first = *mm.first;
            strips[layer].bounds.second = *mm.second;
        }
    }
}

blobs_t WireCell::RayGrid::make_blobs(const Coordinates& coords, const activities_t& activities)
{
    Tiling rc(coords);
    blobs_t blobs;

    for (const auto& activity : activities) {
        if (blobs.empty()) {
            blobs = rc(activity);
        }
        else {
            blobs = rc(blobs, activity);
            if (blobs.empty()) {
                spdlog::trace("RayGrid::make_blobs: lost blobs with {}", activity);
                return blobs_t{};
            }
        }
        drop_invalid(blobs);
    }
    prune(coords, blobs);
    return blobs;
}
