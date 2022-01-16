#ifndef WIRECELLIFACE_IAPPLICATION
#define WIRECELLIFACE_IAPPLICATION

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    /**
     * An application executes something post-configuration.
     */
    class IApplication : public IComponent<IApplication> {
       public:
        virtual ~IApplication();

        /// Implement to run something
        virtual void execute() = 0;
    };

}  // namespace WireCell

#endif
