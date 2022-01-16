#ifndef WIRECELLGEN_WIRESOURCE
#define WIRECELLGEN_WIRESOURCE

#include "WireCellGen/WireParams.h"
#include "WireCellGen/WireGenerator.h"
#include "WireCellIface/IWireSource.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {

    /** A WireCell::IWireSource facade in front of
     * WireCell::WireParams and WireCell::WireGenerator.
     */

    class WireSource : public IWireSource, public IConfigurable {
       public:
        WireSource();
        virtual ~WireSource();

        virtual bool operator()(output_pointer& wires);

        /** Configurable interface.
         */
        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;

       private:
        std::shared_ptr<WireParams> m_params;
        WireGenerator m_wiregen;
    };

}  // namespace WireCell

#endif
