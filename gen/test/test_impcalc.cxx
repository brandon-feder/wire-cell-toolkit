#include "WireCellUtil/Logging.h"
#include "WireCellUtil/Testing.h"
#include <map>
#include <cmath>

using spdlog::debug;
using namespace WireCell;

struct PIR {

    int nwires{21};             // number of wires spanning response
    double pitch{5.0};          // wire pitch in mm
    double impact{0.5};         // impact pitch in mm

    int nimp_per_wire() const {
        return 1 + int(round(pitch/impact))/2;
    }
    double half_extent() const {
        return (nwires/2  + 0.5) * pitch;
    }
    std::pair<int, int> closest_wire_impact(double relpitch) const {
        const int center_wire = nwires / 2;

        const int relwire = int(round(relpitch / pitch));
        const int wire_index = center_wire + relwire;

        const double remainder_pitch = relpitch - relwire * pitch;
        const int impact_index = int(round(remainder_pitch / impact)) + nimp_per_wire() / 2;

        debug("impcalc: rp:{:6.2f} ii:({},{}) relwire:{} remainder:{} (pitch:{:4.1f})",
              relpitch,
              wire_index, impact_index,
              relwire, remainder_pitch, pitch);

        return std::make_pair(wire_index, impact_index);
    }
};

int main()
{
    Log::add_stdout(true, "debug");
    Log::set_level("debug");



    PIR pir;
    Assert(6 == pir.nimp_per_wire());

    for (double rp=-pir.half_extent(); rp<=pir.half_extent(); rp += pir.impact) {
        pir.closest_wire_impact(rp);
    }

    debug("nwires:{} half_extent:{}", pir. nwires, pir.half_extent());
    return 0;
}
