/** An IWirePlane.
 */

#ifndef WIRECELLGEN_WIREPLANE
#define WIRECELLGEN_WIREPLANE

#include "WireCellIface/IWirePlane.h"

namespace WireCell {
    namespace Gen {

        class WirePlane : public IWirePlane {
           public:
            // WirePlane(int ident, IWire::vector wires, Pimpos* pimpos, PlaneImpactResponse* pir);
            WirePlane(int ident, Pimpos* pimpos, const IWire::vector& wires, const IChannel::vector& channels);
            virtual ~WirePlane();

            virtual int ident() const { return m_ident; }

            virtual const Pimpos* pimpos() const { return m_pimpos; }

            /// Access response functions.
            // virtual const PlaneImpactResponse* pir() const { return m_pir; }

            /// Return vector of wire objects ordered by increasing Z.
            virtual const IWire::vector& wires() const { return m_wires; }

            /// Return vector of channel objects ordered by their index
            /// (NOT their channel ident number).
            virtual const IChannel::vector& channels() const { return m_channels; }

           private:
            int m_ident;
            Pimpos* m_pimpos;
            // PlaneImpactResponse* m_pir;
            IWire::vector m_wires;
            IChannel::vector m_channels;
            Ray m_bbox;  // bounding box of wire planes in external coordinates
        };
    }  // namespace Gen

}  // namespace WireCell

#endif
