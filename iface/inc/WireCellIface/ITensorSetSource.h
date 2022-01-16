#ifndef WIRECELL_ITENSORSETSOURCE
#define WIRECELL_ITENSORSETSOURCE

#include "WireCellIface/ITensorSet.h"
#include "WireCellIface/ISourceNode.h"
#include "WireCellUtil/IComponent.h"

namespace WireCell {

    /** A frame source is something that generates IFrames.
     */
    class ITensorSetSource : public ISourceNode<ITensorSet> {
       public:
        typedef std::shared_ptr<ITensorSetSource> pointer;

        virtual ~ITensorSetSource(){};

        // supply:
        // virtual bool operator()(ITensorSet::pointer& frame);
    };

}  // namespace WireCell

#endif