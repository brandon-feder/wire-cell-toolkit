
#include "WireCellIface/IWireSelectors.h"

namespace WireCell {

    wire_selector select_u_wires = WirePlaneSelector(kUlayer, 0, 0);
    wire_selector select_v_wires = WirePlaneSelector(kVlayer, 0, 0);
    wire_selector select_w_wires = WirePlaneSelector(kWlayer, 0, 0);
    wire_selector select_uvw_wires[3] = {select_u_wires, select_v_wires, select_w_wires};
    wire_selector select_all_wires = WirePlaneSelector(kUlayer | kVlayer | kWlayer, 0, 0);

}  // namespace WireCell
