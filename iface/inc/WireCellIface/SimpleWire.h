#ifndef WIRECELLIFACE_SIMPLEWIRE
#define WIRECELLIFACE_SIMPLEWIRE

#include "WireCellIface/IWire.h"

namespace WireCell {

    /** A wire that simply holds all its data. */
    class SimpleWire : public WireCell::IWire {
        WireCell::WirePlaneId m_wpid;
        int m_ident;
        int m_index;
        int m_channel;
        int m_segment;
        WireCell::Ray m_ray;

       public:
        SimpleWire(WireCell::WirePlaneId wpid, int ident, int index, int channel, const WireCell::Ray& ray,
                   int segment = 0)
          : m_wpid(wpid)
          , m_ident(ident)
          , m_index(index)
          , m_channel(channel)
          , m_segment(segment)
          , m_ray(ray)
        {
        }
        virtual ~SimpleWire();

        // this probably could overflow? switch to long int for wire ident?
        int ident() const { return m_ident; }

        WireCell::WirePlaneId planeid() const { return m_wpid; }

        int index() const { return m_index; }

        int channel() const { return m_channel; }

        WireCell::Ray ray() const { return m_ray; }

        int segment() const { return m_segment; }
    };

}  // namespace WireCell

#endif
