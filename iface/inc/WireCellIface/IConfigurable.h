#ifndef WIRECELLIFACE_ICONFIGURABLE
#define WIRECELLIFACE_ICONFIGURABLE

#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/IComponent.h"

namespace WireCell {

    /** Interface by which a class may be configured.
     *
     * Configuration is via a Boost property tree.
     */
    class IConfigurable : virtual public IComponent<IConfigurable> {
       public:
        virtual ~IConfigurable();

        /// Optional, override to return a hard-coded default configuration.
        virtual WireCell::Configuration default_configuration() const { return WireCell::Configuration(); }

        /// Accept a configuration.
        virtual void configure(const WireCell::Configuration& config) = 0;
    };

}  // namespace WireCell
#endif
