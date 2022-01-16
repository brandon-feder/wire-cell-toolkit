#ifndef WIRECELL_IDEPOSOURCE
#define WIRECELL_IDEPOSOURCE

#include "WireCellIface/ISourceNode.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    /** A depo source is a node that generates IDepo objects.
     */
    class IDepoSource : public ISourceNode<IDepo> {
       public:
        typedef std::shared_ptr<IDepoSource> pointer;

        virtual ~IDepoSource();

        virtual std::string signature() { return typeid(IDepoSource).name(); }

        // supply:
        // virtual bool operator()(IDepo::pointer& depo);
    };

}  // namespace WireCell

#endif
