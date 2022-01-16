/**
   A reframer takes makes a "rectangular" frame filled with samples
   from the tagged traces of its input.  This new frame has exactly
   one trace for each channel and each trace is padded to span a
   uniform duration.  These configuration paramters control how the
   new frame is shaped:

   - tags :: list of trace tags to consider.  an empty list means to
     use all traces.  Default: [].

   - tbin :: the number of ticks measured from the *input* frame
     reference time to start each output trace.  Converted to time via
     the input frame's tick, this value will be applied to the output
     frame's reference time.  As such, the "tbin()" method of the
     output traces will return 0.  Default: 0.

   - nticks :: the total number of ticks in the output traces.
     Default: 0.

   - toffset :: an addition time offset arbitrarily added to the
     output frame's reference time.  This time is *asserts some
     fiction* and does not contribute to calculating the output tbin.
     Default: 0.0.

   - fill :: if a needed sample does not exist it will be set to this
     value.  Default: 0.0.

 */

#ifndef WIRECELL_GEN_REFRAMER
#define WIRECELL_GEN_REFRAMER

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellAux/Logger.h"

#include <vector>
#include <string>

namespace WireCell {
    namespace Gen {

        class Reframer : public Aux::Logger,
                         public IFrameFilter, public IConfigurable {
           public:
            Reframer();
            virtual ~Reframer();

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& inframe, output_pointer& outframe);

           private:
            IAnodePlane::pointer m_anode;

            // Consider traces with these tags.  No tags mean all traces.
            std::vector<std::string> m_input_tags;
            // If not empty, apply this tag to output frame
            std::string m_frame_tag;

            double m_toffset, m_fill;
            int m_tbin, m_nticks;

            size_t m_count{0};
        };
    }  // namespace Gen
}  // namespace WireCell
#endif
