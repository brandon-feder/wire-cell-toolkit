#ifndef WIRECELLIFACE_ITRACE
#define WIRECELLIFACE_ITRACE

#include "WireCellIface/IData.h"
#include "WireCellIface/ISequence.h"

#include <vector>

namespace WireCell {

    /** Interface to charge vs time waveform signal on a channel.
     *
     *	A trace is an ordered sequence of charge measurements in
     *	contiguous time bins.
     */
    class ITrace : public IData<ITrace> {
       public:
        /// Sequential collection of charge.
        //
        // fixme: should replace this with a Waveform::realseq_t
        typedef std::vector<float> ChargeSequence;

        virtual ~ITrace();

        /// Return the identifier number for the channel on which this
        /// trace was recorded.
        virtual int channel() const = 0;

        /// Return the time bin relative to some absolute time
        /// (typically the start of the frame) at which the first
        /// ADC/charge in the trace was digitized (leading bin edge).
        virtual int tbin() const = 0;

        /// Return the contiguous adc/charge measurements on the
        /// channel starting at tbin.
        virtual const ChargeSequence& charge() const = 0;
    };

}  // namespace WireCell

#endif
