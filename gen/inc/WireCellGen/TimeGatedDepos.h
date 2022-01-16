#ifndef WIRECELL_GEN_TIMEGATEDDEPOS
#define WIRECELL_GEN_TIMEGATEDDEPOS

#include "WireCellIface/IDrifter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellUtil/Units.h"

#include <tuple>
#include <vector>

namespace WireCell {

    namespace Gen {

        /**
           TimeGatedDepos outputs depos which pass a time based
           selection.

           The selection is defined as a time gate expressed as a
           start time and a duration.

           If a period is provided than the time gate is advanced by
           the configured period each time an EOS is received.

           It operates in one of accept or reject modes.  To be output
           in the former mode depos must have times within the gate,
           in the latter they must not.

           The gate is half inclusive.  Depo landing exactly at the
           end of the gate is considered outside the gate.
         */
        class TimeGatedDepos : public IDrifter, public IConfigurable {
           public:
            TimeGatedDepos();
            virtual ~TimeGatedDepos();
            virtual bool operator()(const input_pointer& depo, output_queue& outq);
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            bool m_accept;
            double m_period;
            double m_start;
            double m_duration;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
