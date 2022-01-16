#ifndef WIRECELL_ZSENDEDTRACE
#define WIRECELL_ZSENDEDTRACE

#include "WireCellIface/ITrace.h"

#include <map>
#include <vector>

namespace WireCell {

    /** This concrete trace is filled by time bin and charge.
     *
     * It provides the results of the filling such that the ChargeSequence
     * is trivially (exactly) zero suppressed but only at the ends.  Any
     * zeros bounded by non-zero charge are kept.
     */
    class ZSEndedTrace : public ITrace {
        int m_chid, m_nbins;
        std::map<int, float> m_chqmap;
        mutable ChargeSequence m_charge;

       public:
        ZSEndedTrace(int chid, int nbins = 0);

        /// used to fill.
        void operator()(int tbin, float charge);

        virtual int channel() const;

        virtual int tbin() const;

        virtual const ChargeSequence& charge() const;
    };

}  // namespace WireCell

#endif
