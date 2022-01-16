#ifndef WIRECELL_IDEPOSETSOURCE
#define WIRECELL_IDEPOSETSOURCE

#include "WireCellIface/ISourceNode.h"
#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    /** A source node that generates IDepoSet objects.
     */
    class IDepoSetSource : public ISourceNode<IDepoSet> {
       public:
        typedef std::shared_ptr<IDepoSetSource> pointer;

        virtual ~IDepoSetSource();

        virtual std::string signature() { return typeid(IDepoSetSource).name(); }

        // supply:
        // virtual bool operator()(IDepoSet::pointer& depo);
    };

}  // namespace WireCell

#endif
