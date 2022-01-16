#ifndef WIRECELLIFACE_ITERMINAL
#define WIRECELLIFACE_ITERMINAL

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class ITerminal : virtual public IComponent<ITerminal> {
       public:
        virtual ~ITerminal();

        /// This will be called after an overall "execution".
        /// Implementing components may use this method to dealocate
        /// any resources which need explicit freeing.
        virtual void finalize() = 0;
    };

}  // namespace WireCell
#endif
