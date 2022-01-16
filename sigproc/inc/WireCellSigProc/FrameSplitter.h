#ifndef WIRECELLSIGPROC_FRAMESPLITTER
#define WIRECELLSIGPROC_FRAMESPLITTER

#include "WireCellIface/IFrameSplitter.h"

namespace WireCell {
    namespace SigProc {

        // fixme/note: this class is generic and nothing to do with
        // sigproc per se.  There are a few such frame related nodes
        // also in gen.  They should be moved into some neutral package.
        class FrameSplitter : public WireCell::IFrameSplitter {
           public:
            FrameSplitter();
            virtual ~FrameSplitter();

            virtual bool operator()(const input_pointer& in, output_tuple_type& out);
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
