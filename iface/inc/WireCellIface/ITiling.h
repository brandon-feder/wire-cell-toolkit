/**  A tiling consumes a time slice of channel samples and produces a set of blobs from it.
 */

#ifndef WIRECELL_ITILING
#define WIRECELL_ITILING

#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/ISlice.h"
#include "WireCellIface/IBlobSet.h"

namespace WireCell {

    class ITiling : public IFunctionNode<ISlice, IBlobSet> {
       public:
        typedef std::shared_ptr<ITiling> pointer;

        virtual ~ITiling();

        virtual std::string signature() { return typeid(ITiling).name(); }

        // supply:
        // virtual bool operator()(const input_pointer& in, output_pointer& out);
    };
}  // namespace WireCell

#endif
