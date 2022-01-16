#ifndef WIRECELL_ICOMPONENT
#define WIRECELL_ICOMPONENT

#include "WireCellUtil/Interface.h"

#include <vector>

namespace WireCell {

    /** A component calls out a particular interface as being
     * identified as important to the system.  Inheriting from
     * IComponent follows the CRTP pattern:
     *
     * class IMyComponent : public IComponent<IMyComponent> {
     *   public: void mymethod() = 0;
     * };
     *
     * class MyConcreteComponent : public IMyComponent {
     *   public: void mymethod();
     * };
     *
     * IMyComponent::pointer mycomp(new MyConcreteComponent);
     * mycomp->mymethod();
     *
     * See WireCell::NamedFactory for one creation pattern that works
     * with IComponents.
     */
    template <class Type>  // crtp
    class IComponent : virtual public Interface {
       public:
        /// Access subclass facet by pointer.
        typedef std::shared_ptr<Type> pointer;

        /// Vector of shared pointers.
        typedef std::vector<pointer> vector;

        virtual ~IComponent(){};
    };

}  // namespace WireCell

#endif
