#ifndef WIRECELL_IFACE_TEST_MYCELL
#define WIRECELL_IFACE_TEST_MYCELL

#include "WireCellIface/ICell.h"

class MyCell : public WireCell::ICell {
    int m_ident;
    double m_area;

   public:
    MyCell(int i, double a)
      : m_ident(i)
      , m_area(a)
    {
    }
    virtual ~MyCell() {}
    int ident() const { return m_ident; }

    double area() const { return m_area; }

    WireCell::Point center() const { return WireCell::Point(); }

    WireCell::PointVector corners() const { return WireCell::PointVector(); }

    WireCell::IWireVector wires() const { return WireCell::IWireVector(); }
};
#endif
