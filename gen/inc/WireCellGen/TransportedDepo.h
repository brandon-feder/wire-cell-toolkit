#ifndef WIRECELL_NAV_TRANSPORTEDDEPO
#define WIRECELL_NAV_TRANSPORTEDDEPO

#include "WireCellIface/IDepo.h"

namespace WireCell {
    namespace Gen {

        // This is like SimpleDepo but requires a prior from which it
        // gets the charge.
        class TransportedDepo : public WireCell::IDepo {
            WireCell::IDepo::pointer m_from;
            WireCell::Point m_pos;
            double m_time;

           public:
            TransportedDepo(const WireCell::IDepo::pointer& from, double location, double velocity)
              : m_from(from)
              , m_pos(from->pos())
            {
                double dx = m_pos.x() - location;
                m_pos.x(location);
                m_time = from->time() + dx / velocity;
            }
            virtual ~TransportedDepo(){};

            virtual const WireCell::Point& pos() const { return m_pos; }
            virtual double time() const { return m_time; }
            virtual double charge() const { return m_from->charge(); }
            virtual int id() const { return m_from->id(); }
            virtual int pdg() const { return m_from->pdg(); }
            virtual double energy() const { return m_from->energy(); }
            virtual WireCell::IDepo::pointer prior() const { return m_from; }
        };

    }  // namespace Gen
}  // namespace WireCell
#endif
