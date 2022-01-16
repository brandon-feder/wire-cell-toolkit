#ifndef WIRECELL_IWIRESUMMARIZER
#define WIRECELL_IWIRESUMMARIZER

#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IWireSummary.h"

namespace WireCell {

    /** A wire summarizer is a function node which produces a wire summary object for a collection of wires.
     */
    class IWireSummarizer : public IFunctionNode<IWire::vector, IWireSummary> {
       public:
        virtual ~IWireSummarizer();

        virtual std::string signature() { return typeid(IWireSummarizer).name(); }
    };
}  // namespace WireCell

#endif
