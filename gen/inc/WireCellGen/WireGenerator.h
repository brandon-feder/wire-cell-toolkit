#ifndef WIRECELLGEN_WIREGENERATOR
#define WIRECELLGEN_WIREGENERATOR

#include "WireCellIface/IWireGenerator.h"
#include "WireCellIface/IWire.h"

#include <deque>

namespace WireCell {

    /** A source of wire (segment) geometry as generated from parameters.
     *
     * All wires in one plane are constructed to be parallel to
     * one-another and to be equally spaced between neighbors and
     * perpendicular to the drift direction.
     */

    class WireGenerator : public IWireGenerator {
       public:
        WireGenerator();
        virtual ~WireGenerator();

        virtual bool operator()(const input_pointer& params, output_pointer& wires);
    };

}  // namespace WireCell

#endif
