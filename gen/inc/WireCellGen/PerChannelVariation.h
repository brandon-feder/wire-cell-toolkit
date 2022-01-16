/** This component will output a "ch-by-ch variation" trace for each input
 * trace. Reference to "Misconfigure"
 *
 * It does this by filtering out an assumed electronics response
 * function and applying a new one from ch-by-ch electronics calibration.
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

#ifndef WIRECELLGEN_PERCHANNELVARIATION_
#define WIRECELLGEN_PERCHANNELVARIATION_

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IChannelResponse.h"
#include "WireCellUtil/Waveform.h"

#include <unordered_set>

namespace WireCell {
    namespace Gen {

        class PerChannelVariation : public IFrameFilter, public IConfigurable {
           public:
            PerChannelVariation();
            virtual ~PerChannelVariation();

            // IFrameFilter
            virtual bool operator()(const input_pointer& in, output_pointer& out);

            // IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& cfg);

           private:
            std::string m_per_chan_resp;
            WireCell::IChannelResponse::pointer m_cr;
            int m_nsamples;
            WireCell::Waveform::realseq_t m_from;
            bool m_truncate;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
