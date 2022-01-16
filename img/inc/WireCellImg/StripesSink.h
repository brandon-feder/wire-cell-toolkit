#ifndef WIRECELLIMG_STRIPESSINK
#define WIRECELLIMG_STRIPESSINK

#include "WireCellIface/ISinkNode.h"
#include "WireCellIface/IStripeSet.h"

namespace WireCell {

    // don't bother with an intermediate stripe set sink node interface...
    class IStripeSetSink : public ISinkNode<IStripeSet> {
       public:
        typedef std::shared_ptr<IStripeSetSink> pointer;

        virtual ~IStripeSetSink() {}

        virtual std::string signature() { return typeid(IStripeSetSink).name(); }
    };

    namespace Img {

        class StripesSink : public IStripeSetSink {
           public:
            virtual ~StripesSink();

            virtual bool operator()(const IStripeSet::pointer& ss);
        };

    }  // namespace Img
}  // namespace WireCell

#endif
