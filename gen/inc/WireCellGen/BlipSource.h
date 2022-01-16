/** A blip source produces individual energy depositions in various
 * ways.
 */

#ifndef WIRECELLGEN_BLIPSOURCE
#define WIRECELLGEN_BLIPSOURCE

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IRandom.h"

namespace WireCell {
    namespace Gen {

        class BlipSource : public IDepoSource, public IConfigurable {
           public:
            BlipSource();
            virtual ~BlipSource();

            /// IDepoSource
            virtual bool operator()(IDepo::pointer& depo);

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            // Internal base class for something that makes a scalar
            struct ScalarMaker {
                virtual double operator()() = 0;
                virtual ~ScalarMaker(){};
            };
            // Internal base class for something that makes a scalar
            struct PointMaker {
                virtual Point operator()() = 0;
                virtual ~PointMaker(){};
            };

           private:
            std::string m_rng_tn;
            IRandom::pointer m_rng;

            double m_time, m_stop;
            ScalarMaker *m_ene, *m_tim;
            PointMaker* m_pos;
            int m_blip_count;
            bool m_eos;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
