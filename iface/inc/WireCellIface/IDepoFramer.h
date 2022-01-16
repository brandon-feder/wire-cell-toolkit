/**

   A depo framer takes in a collection of depositions and produces a
   single frame that contains their associated signal waveforms.
   Excess depos may be dropped.

 */

#ifndef WIRECELLIFACE_IDEPOFRAMER
#define WIRECELLIFACE_IDEPOFRAMER

#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    class IDepoFramer : public IFunctionNode<IDepoSet, IFrame> {
       public:
        virtual ~IDepoFramer();

        virtual std::string signature() { return typeid(IDepoFramer).name(); }

        // implement:
        // virtual bool operator()(const input_pointer& in, output_pointer& out) = 0;
    };
}  // namespace WireCell

#endif
