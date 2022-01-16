// This implements an IChannel data interface.

#ifndef WIRECELL_IFACE_SIMPLECHANNEL
#define WIRECELL_IFACE_SIMPLECHANNEL

#include "WireCellIface/IChannel.h"

namespace WireCell {

    class SimpleChannel : public IChannel {
       public:
        SimpleChannel(int ident = -1, int index = -1, const IWire::vector& wires = IWire::vector());
        virtual ~SimpleChannel();

        // IChannel interface
        virtual int ident() const;
        virtual int index() const;
        virtual const IWire::vector& wires() const;

        // Personal interface for creator.
        void add(const IWire::pointer& wire);
        void set_index(int ind);

       private:
        int m_ident, m_index;
        IWire::vector m_wires;
    };

}  // namespace WireCell

#endif
