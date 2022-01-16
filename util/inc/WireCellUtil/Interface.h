#ifndef WIRECELL_INTERFACE
#define WIRECELL_INTERFACE

#include <memory>

namespace WireCell {

    /** An interface defines some facet of functionality which is
     * accessed by the type "pointer".
     *
     * See also WireCell::IComponent.
     */
    class Interface {
       public:
        // Interfaces are accessed by non-const shared pointers.
        typedef std::shared_ptr<Interface> pointer;

        virtual ~Interface();
    };

}  // namespace WireCell

#endif
