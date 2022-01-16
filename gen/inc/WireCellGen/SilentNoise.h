#ifndef WIRECELLGEN_SILENTNOISE
#define WIRECELLGEN_SILENTNOISE

#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Gen {

        /// A source of "noise" which has no noise.  It's used just as
        /// an trivial example which real noise models may copy.
        /// Although it inherits from IConfigurable, it's not really.
        /// Again, just giving an example.
        class SilentNoise : public IFrameSource, public IConfigurable {
           public:
            SilentNoise();
            virtual ~SilentNoise();

            // configurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // frame source
            virtual bool operator()(output_pointer& out);

           private:
            int m_count;
            int m_noutputs, m_nchannels;
            std::string m_traces_tag;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
