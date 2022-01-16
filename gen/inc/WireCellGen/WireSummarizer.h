#ifndef WIRECELL_WIRESUMMARIZER
#define WIRECELL_WIRESUMMARIZER

#include "WireCellIface/IWireSummarizer.h"
#include "WireCellIface/IWireSummary.h"

#include <deque>

namespace WireCell {

    class WireSummarizer : public IWireSummarizer {
       public:
        WireSummarizer();
        virtual ~WireSummarizer();

        virtual bool operator()(const input_pointer& in, output_pointer& out);
    };

}  // namespace WireCell

#endif
