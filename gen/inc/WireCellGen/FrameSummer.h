// A frame summer can "add" two frames together, possibly limiting
// them by tag.

#ifndef WIRECELL_GEN_FRAMESUMMER
#define WIRECELL_GEN_FRAMESUMMER

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFrameJoiner.h"

namespace WireCell {
    namespace Gen {
        class FrameSummer : public IFrameJoiner, public IConfigurable {
           public:
            FrameSummer();
            virtual ~FrameSummer();

            // IJoinNode
            virtual bool operator()(const input_tuple_type& intup, output_pointer& out);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            double m_toffset;
            bool m_align;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
