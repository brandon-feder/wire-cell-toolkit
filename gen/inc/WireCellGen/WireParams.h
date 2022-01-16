#ifndef WIRECELL_WIREPARAMS
#define WIRECELL_WIREPARAMS

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IWireParameters.h"
#include "WireCellUtil/Units.h"

namespace WireCell {

    /** Embody parameters describing a triple of wire planes and
     * provide a configurable interface. */
    class WireParams : public IWireParameters, public IConfigurable {
       public:
        /// Directly set the fundamental parameters.
        /// See WireCell::IWireParameters.
        void set(const Ray& bounds, const Ray& U, const Ray& V, const Ray& W);

        /** Set the wire parameters simply.
         *
         * This method gives a simpler, more restricted interface to
         * setting the wire parameter.  It follows the same
         * conventions as the one above but assumes:
         *
         * - The bounding box is centered on the origin.
         *
         * - One wire of each plane intersects the X-axis.
         *
         * - The magnitudes of the U and V angles are equal.
         *
         * - The pitches of all three planes are the same.
         *
         * - The X values W, V and U planes evening distributed on the
         * positive half of the X covered by the bounding box.
         *
         * \param dx, dy, dz are the full widths of the bounding box
         * in the associated direction.
         *
         * \param pitch is the perpendicular distance between two
         * adjacent wires in a plane.
         *
         * \param angle is the absolute angular distance from the U
         * and V wires and the Y-axis.
         */
        void set(double dx = 10 * units::mm, double dy = 1 * units::meter, double dz = 1 * units::meter,
                 double pitch = 10 * units::mm, double angle = 60.0 * units::degree);

        /** Provide access to the rays which were used to define the wires. */
        const Ray& bounds() const;
        const Ray& pitchU() const;
        const Ray& pitchV() const;
        const Ray& pitchW() const;

        /** Configurable interface.
         */
        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;

        WireParams();
        virtual ~WireParams();

       private:
        Ray m_bounds, m_pitchU, m_pitchV, m_pitchW;
    };
}  // namespace WireCell

#endif
