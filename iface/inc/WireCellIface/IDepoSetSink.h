#ifndef WIRECELL_IDEPOSETSINK
#define WIRECELL_IDEPOSETSINK

#include "WireCellIface/ISinkNode.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    /** A node that consumes IDepoSet objects.
     */
    class IDepoSetSink : public ISinkNode<IDepoSet> {
       public:
        virtual ~IDepoSetSink();

        virtual std::string signature() { return typeid(IDepoSetSink).name(); }

        // supply:
        // virtual bool operator()(const IDepoSet::pointer& depos);
    };

}  // namespace WireCell

#endif
