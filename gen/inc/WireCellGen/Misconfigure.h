/** This component will output a "misconfigured" trace for each input
 * trace.
 *
 * It does this by filtering out an assumed electronics response
 * function and applying a new one.
 *
 * Note, traces are "misconfigured" independently even if multiple
 * traces exist on the same channel.
 *
 * By default the output traces will be sized larger than input by
 * nsamples-1.  If the "truncated" option is true then the output
 * trace will be truncated to match the input size.  this may cut off
 * signal for traces smaller than the time where the electronics
 * response functions are finite.
 *
 * This component does not honor frame/trace tags.  No tags will be
 * considered on input and none are placed on output.
 *
 */

#ifndef WIRECELLGEN_MISCONFIGURE
#define WIRECELLGEN_MISCONFIGURE

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Waveform.h"

#include <unordered_set>

namespace WireCell {
    namespace Gen {

        class Misconfigure : public IFrameFilter, public IConfigurable {
           public:
            Misconfigure();
            virtual ~Misconfigure();

            // IFrameFilter
            virtual bool operator()(const input_pointer& in, output_pointer& out);

            // IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& cfg);

           private:
            Waveform::realseq_t m_from, m_to;
            bool m_truncate;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
