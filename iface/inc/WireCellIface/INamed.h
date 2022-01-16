/**
   A "named" component.

   This interface allows setting and getting an instance name.

   When used from WireCell::Main, INamed::set_name() will be called
   before configure() (if also an IConfigurable).
 */

#ifndef WIRECELLIFACE_INAMED
#define WIRECELLIFACE_INAMED

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class INamed : virtual public IComponent<INamed> {
       public:
        virtual ~INamed();

        virtual void set_name(const std::string& name) = 0;
        virtual std::string get_name() const = 0;
    };

}

#endif
