#ifndef WIRECELL_IFACTORY
#define WIRECELL_IFACTORY

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class IFactory : public IComponent<IFactory> {
       public:
        virtual ~IFactory();

        /// Create an instance of what we know how to create.
        virtual Interface::pointer create() = 0;

        /// Return existing instance or nullptr if not found.
        virtual Interface::pointer find(const std::string& name) = 0;
    };

    class INamedFactory : public IFactory {
       public:
        typedef std::shared_ptr<INamedFactory> pointer;

        virtual ~INamedFactory();

        /// Set name of class this factory can make
        virtual void set_classname(const std::string& name) = 0;
        /// Access name of class this factory can make
        virtual const std::string& classname() = 0;

        /// Create an instance by name.
        virtual Interface::pointer create(const std::string& name) = 0;
        using IFactory::create;

        /// Return existing instance or nullptr if not found.
        virtual Interface::pointer find(const std::string& name) = 0;
    };

}  // namespace WireCell

#endif
