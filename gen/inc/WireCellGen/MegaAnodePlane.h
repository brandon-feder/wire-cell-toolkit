// larweirecell::FrameSaver needs a general channel-to-wireplane
// resolver fixme: this is a temporary solution, only a few functions
// are useful, otherwise, please do NOT use the other virtual fuctions
// inherited from IAnodePlane

#ifndef WIRECELLGEN_MEGAANODEPLANE
#define WIRECELLGEN_MEGAANODEPLANE

#include "WireCellGen/AnodePlane.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IConfigurable.h"
#include <vector>

namespace WireCell {
    namespace Gen {

        class MegaAnodePlane : public IAnodePlane, public IConfigurable {
           public:
            // MegaAnodePlane();
            virtual ~MegaAnodePlane() {}

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // IAnodePlane interface

            // Actually implemented
            virtual WirePlaneId resolve(int channel) const;
            virtual std::vector<int> channels() const;
            virtual IChannel::pointer channel(int chident) const;
            virtual IWire::vector wires(int channel) const;

            // The number of faces is kind of a ill defined quantity
            // for the mega anode plane.
            virtual int nfaces() const { return m_anodes[0]->nfaces(); }

            // Implemented with dummies
            virtual int ident() const { return -1; }
            virtual IAnodeFace::pointer face(int ident) const { return nullptr; }
            virtual IAnodeFace::vector faces() const { return IAnodeFace::vector(); }

           private:
            std::vector<IAnodePlane::pointer> m_anodes;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
