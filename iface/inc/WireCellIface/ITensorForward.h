/** An interface to a "forward" operator on a tensor set */

#ifndef WIRECELL_ITENSORFORWARD
#define WIRECELL_ITENSORFORWARD

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/ITensorSet.h"

namespace WireCell {
    class ITensorForward : public IComponent<ITensorForward> {
      public:
        virtual ~ITensorForward();

        virtual ITensorSet::pointer forward(const ITensorSet::pointer& input) const = 0;
    };
}  // namespace WireCell

#endif  // WIRECELL_ITENSORFORWARD
