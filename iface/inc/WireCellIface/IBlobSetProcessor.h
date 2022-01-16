/** IBlobSetProcessor consumes blob sets and sometimes produces them.
 */
#ifndef WIRECEEL_IBLOBSETPROCESSOR_H
#define WIRECEEL_IBLOBSETPROCESSOR_H

#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IBlobSet.h"

namespace WireCell {

    class IBlobSetProcessor : public IQueuedoutNode<IBlobSet, IBlobSet> {
       public:
        virtual ~IBlobSetProcessor();
        typedef std::shared_ptr<IBlobSetProcessor> pointer;

        virtual std::string signature() { return typeid(IBlobSetProcessor).name(); }

        /// supply:
        // virtual bool operator()(const input_pointer& in, output_queue& outq) = 0;
    };

}  // namespace WireCell

#endif /* WIRECEEL_IBLOBSETPROCESSOR_H */
