// This component produces an output frame which consists of traces
// from the two input frames that have been merged together based on
// tags and a rule.

#ifndef WIRECELL_SIGPROC_FRAMEMERGER
#define WIRECELL_SIGPROC_FRAMEMERGER

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFrameJoiner.h"
#include "WireCellUtil/Configuration.h"

namespace WireCell {
    namespace SigProc {
        class FrameMerger : public IFrameJoiner, public IConfigurable {
           public:
            FrameMerger();
            virtual ~FrameMerger();

            // IJoinNode
            virtual bool operator()(const input_tuple_type& intup, output_pointer& out);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            Configuration m_cfg;
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
