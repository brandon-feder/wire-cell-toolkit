#ifndef WIRECELLGEN_DUMPFRAMES
#define WIRECELLGEN_DUMPFRAMES

#include "WireCellIface/IFrameSink.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Gen {

        class DumpFrames : public IFrameSink {
           public:
            DumpFrames();
            virtual ~DumpFrames();

            /// Do something thrilling with a frame.
            virtual bool operator()(const IFrame::pointer& frame);

           private:
            Log::logptr_t log;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
