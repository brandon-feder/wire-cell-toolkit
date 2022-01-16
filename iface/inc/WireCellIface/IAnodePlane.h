/**
   Provides information about an "anode plane" which consists of a
   number of parallel wire planes as IWirePlane objects.

   fixme: this has become kind of a kitchen sink.

 */

#ifndef WIRECELLIFACES_IANODEPLANE
#define WIRECELLIFACES_IANODEPLANE

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IAnodeFace.h"
#include "WireCellIface/IWire.h"
#include "WireCellIface/IChannel.h"
#include "WireCellIface/WirePlaneId.h"

namespace WireCell {

    class IAnodePlane : public IComponent<IAnodePlane> {
       public:
        virtual ~IAnodePlane();

        /// Return the ident number of this plane.
        virtual int ident() const = 0;

        /// Return number of faces (eg, MicroBooNE=1, DUNE=2)
        virtual int nfaces() const = 0;

        /// Return a anode face by its ident number.
        virtual IAnodeFace::pointer face(int ident) const = 0;

        /// Return all faces
        virtual IAnodeFace::vector faces() const = 0;

        /// Resolve a channel ident number to a WirePlaneId.
        virtual WirePlaneId resolve(int channel) const = 0;

        /// Return a collection of all channels.
        virtual std::vector<int> channels() const = 0;

        /// Return an IChannel with the associated channel ID.
        virtual IChannel::pointer channel(int chident) const = 0;

        /// Return all wires connected into the given channel ident number
        virtual IWire::vector wires(int chident) const = 0;
    };

}  // namespace WireCell

#endif
