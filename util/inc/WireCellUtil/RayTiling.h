/** Define a tiling of overlapping sets rays.

    This is part of Ray Grid.

    See the WCT manual for details.
 */

#ifndef WIRECELL_RAYTILING
#define WIRECELL_RAYTILING

#include <map>
#include <vector>
#include <string>

#include "WireCellUtil/RayGrid.h"

namespace WireCell {

    namespace RayGrid {

        struct Strip {
            // The index of the layer this strip is in.
            layer_index_t layer;

            // The pitch indices bounding the strip in its layer pitch direction.
            grid_range_t bounds;

            crossing_t addresses() const
            {
                return std::make_pair(coordinate_t{layer, bounds.first}, coordinate_t{layer, bounds.second});
            }

            bool in(grid_index_t pitch_index) const
            {
                return (bounds.first <= pitch_index) and (pitch_index < bounds.second);
            }
            bool on(grid_index_t pitch_index) const
            {
                return (bounds.first <= pitch_index) and (pitch_index <= bounds.second);
            }
        };
        typedef std::vector<Strip> strips_t;

        // Activity represents an absolutly positioned span of pitch
        // indices which may contain some measure of some kind of
        // activity (ie, an ADC sum measured in the channels fed by
        // wires represented by the rays).  The activity may be
        // indexed by pitch index.  Each index n is considered to
        // contain the activity from its pitch up to but not including
        // the pitch of the n+1 bin.
        class Activity {
           public:
            typedef double value_t;
            typedef std::vector<value_t> vector_t;
            typedef vector_t::const_iterator iterator_t;
            typedef std::pair<iterator_t, iterator_t> range_t;
            typedef std::vector<range_t> ranges_t;

            // Create empty activity
            Activity(layer_index_t layer);

            // Create an activity from a range of some vector starting
            // at given offset in the enumeration of pitch indices.
            Activity(layer_index_t layer, const range_t& span, int offset = 0, double threshold = 0.0);

            // Create an activity starting at offset and with each
            // subsequent span elements set to value.
            Activity(layer_index_t layer, size_t span, double value, int offset = 0, double threshold = 0.0);

            // Produce a subspan activity between pitch indices [pi1, pi2)
            Activity subspan(int pi_begin, int pi_end) const;

            layer_index_t layer() const { return m_layer; }

            iterator_t begin() const;
            iterator_t end() const;

            bool empty() const;

            // lil helpers

            int pitch_index(const iterator_t& it) const;

            // Make a strip from a sub span of the current activity.
            Strip make_strip(const range_t& subspan) const;

            // Return strips bounding contiguous positive activity
            // subspans.
            strips_t make_strips() const;

            // Return all subspans with activities
            ranges_t active_ranges() const;

            int offset() const { return m_offset; }
            std::string as_string() const;

           private:
            vector_t m_span;
            layer_index_t m_layer;
            int m_offset;
            double m_threshold;
        };
        typedef std::vector<Activity> activities_t;

        class Blob {
           public:
            void add(const Coordinates& coords, const Strip& strip);

            const strips_t& strips() const { return m_strips; }
            strips_t& strips() { return m_strips; }

            // Blob corners are pair-wise ray crossing points which
            // are contained by all strips.
            const crossings_t& corners() const;

            bool valid() const
            {
                size_t nstrips = m_strips.size();
                if (nstrips == 0) {
                    return false;
                }  // empty
                if (nstrips == 1) {
                    return true;
                }  // no corners expected
                return corners().size() > 0;
            }

            std::string as_string() const;

           private:
            strips_t m_strips;
            crossings_t m_corners;
        };

        // A collection of blobs.
        typedef std::vector<Blob> blobs_t;

        class Tiling {
           public:
            Tiling(const Coordinates& coords);

            // Return a new activity which is shrunk to fall into the shadow of the blob.
            Activity projection(const Blob& blob, const Activity& activity);

            // Tile activity from initial layer into blobs.
            blobs_t operator()(const Activity& activity);

            // Refine existing blobs with the activity in a new layer.
            blobs_t operator()(const blobs_t& prior, const Activity& activity);

           private:
            const Coordinates& m_coords;
        };

        /// free functions

        // Remove any invalid blobs, return number removed.
        size_t drop_invalid(blobs_t& blobs);

        // Visit each blob and prune away any portions of strips which
        // are outside the corners.  These vestigle strip portions can
        // result when another layer provides a corner inside the
        // strip in question.
        void prune(const Coordinates& coords, blobs_t& blobs);

        // One stop shopping to generate blobs from activity
        blobs_t make_blobs(const Coordinates& coords, const activities_t& activities);

        inline std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Strip& s)
        {
            os << "<strip L" << s.layer << " pind:[" << s.bounds.first << "," << s.bounds.second << "]>";
            return os;
        }

        inline std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Activity& a)
        {
            int b = a.pitch_index(a.begin()), e = a.pitch_index(a.end());
            auto strips = a.make_strips();
            os << "<activity L" << a.layer() << " " << strips.size() << " strips over pind:[" << b << "," << e << "]>";
            return os;
        }

        inline std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::Blob& b)
        {
            os << "<blob " << b.strips().size() << " strips, " << b.corners().size() << " corners>";
            return os;
        }

        inline std::ostream& operator<<(std::ostream& os, const WireCell::RayGrid::crossing_t& c)
        {
            os << "<corner [{L" << c.first.layer << ",G" << c.first.grid << "},"
               << "{L" << c.second.layer << ",G" << c.second.grid << "}]>";
            return os;
        }

    }  // namespace RayGrid
}  // namespace WireCell

#endif
