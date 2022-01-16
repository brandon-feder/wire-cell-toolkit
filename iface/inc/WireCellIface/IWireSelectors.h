/**
   Some wire selectors.

   Fixme: this isn't really appropriate for an interface package.

 */

#ifndef WIRECELL_WIRESELECTORS
#define WIRECELL_WIRESELECTORS

#include "WireCellIface/IWire.h"
#include <boost/function.hpp>

namespace WireCell {

    typedef boost::function<bool(IWire::pointer)> wire_selector;

    /// Select wires by plane (and apa/face)
    struct WirePlaneSelector {
        int layers, face, apa;

        WirePlaneSelector(int layer_mask, int face = 0, int apa = 0)
          : layers(layer_mask)
          , face(face)
          , apa(apa)
        {
        }

        bool operator()(IWire::pointer wire)
        {
            WirePlaneId ident = wire->planeid();

            if (layers && !(layers & ident.ilayer())) {
                return false;
            }
            if (apa >= 0 && ident.apa() != apa) {
                return false;
            }
            if (face >= 0 && ident.face() != face) {
                return false;
            }

            return true;
        }
    };

    // Select all wires on default apa/face
    extern wire_selector select_all_wires;

    // Select one plane on default apa/face
    extern wire_selector select_u_wires;
    extern wire_selector select_v_wires;
    extern wire_selector select_w_wires;
    extern wire_selector select_uvw_wires[3];

}  // namespace WireCell

#endif
