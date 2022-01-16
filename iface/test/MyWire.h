#ifndef WIRECELLIFACE_TEST_MYWIRE
#define WIRECELLIFACE_TEST_MYWIRE

#include "WireCellIface/IWire.h"
#include "WireCellUtil/Point.h"

class MyWire : public WireCell::IWire {
    WireCell::WirePlaneId m_wpid;
    int m_index;
    WireCell::Ray m_ray;

   public:
    MyWire(WireCell::WirePlaneLayer_t layer, int index, const WireCell::Ray& ray)
      : m_wpid(layer)
      , m_index(index)
      , m_ray(ray)
    {
    }
    virtual ~MyWire() {}

    int ident() const
    {
        int iplane = 1 + m_wpid.index();
        return iplane * 100000 + m_index;
    }

    WireCell::WirePlaneId planeid() const { return m_wpid; }

    int index() const { return m_index; }

    int channel() const { return ident(); }

    WireCell::Ray ray() const { return m_ray; }

    int segment() const { return 0; }
};

#endif
