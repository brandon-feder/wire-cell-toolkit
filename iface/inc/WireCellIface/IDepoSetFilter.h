#ifndef WIRECELL_IDEPOSETFILTER
#define WIRECELL_IDEPOSETFILTER

#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    /** DepoSet -> DepoSet
     */
    class IDepoSetFilter : public IFunctionNode<IDepoSet, IDepoSet> {
       public:
        virtual ~IDepoSetFilter();

        typedef std::shared_ptr<IDepoSetFilter> pointer;

        virtual std::string signature() { return typeid(IDepoSetFilter).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };
}  // namespace WireCell

#endif
