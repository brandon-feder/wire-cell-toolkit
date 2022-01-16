#ifndef WIRECELLTBB_TBB_MOCK
#define WIRECELLTBB_TBB_MOCK

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IDrifter.h"
#include "WireCellIface/IDepoSink.h"
#include "WireCellIface/SimpleDepo.h"
#include "WireCellUtil/Units.h"

#include <iostream>

namespace WireCellTbb {

    class MockDepoSource : public WireCell::IDepoSource {
        int m_count;
        const int m_maxdepos;

       public:
        MockDepoSource(int maxdepos = 10000)
          : m_count(0)
          , m_maxdepos(maxdepos)
        {
        }
        virtual ~MockDepoSource() {}

        virtual bool operator()(output_pointer& out)
        {
            if (m_count > m_maxdepos) {
                return false;
            }
            ++m_count;
            double dist = m_count * WireCell::units::millimeter;
            double time = m_count * WireCell::units::microsecond;
            WireCell::Point pos(dist, dist, dist);
            out = WireCell::IDepo::pointer(new WireCell::SimpleDepo(time, pos));
            std::cerr << "Source: " << out->time() / WireCell::units::millimeter << std::endl;
            return true;
        }
    };

    class MockDrifter : public WireCell::IDrifter {
        std::deque<input_pointer> m_depos;

       public:
        virtual ~MockDrifter() {}

        virtual bool operator()(const input_pointer& in, output_queue& outq)
        {
            m_depos.push_back(in);

            // simulate some buffering condition
            size_t n_to_keep = 2;
            if (!in) {
                n_to_keep = 0;
            }

            while (m_depos.size() > n_to_keep) {
                auto depo = m_depos.front();
                m_depos.pop_front();
                outq.push_back(depo);
                std::cerr << "Drift: " << depo->time() / WireCell::units::millimeter << std::endl;
            }

            return true;
        }
    };

    class MockDepoSink : public WireCell::IDepoSink {
       public:
        virtual ~MockDepoSink() {}
        virtual bool operator()(const input_pointer& depo)
        {
            std::cerr << "Sink: " << depo->time() / WireCell::units::millimeter << std::endl;
            return true;
        }
    };

}  // namespace WireCellTbb
#endif
