#ifndef WIRECELLAPPS_NODEDUMPER
#define WIRECELLAPPS_NODEDUMPER

#include "WireCellIface/IApplication.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Configuration.h"

namespace WireCellApps {

    class NodeDumper : public WireCell::IApplication, public WireCell::IConfigurable {
        WireCell::Configuration m_cfg;

       public:
        NodeDumper();
        virtual ~NodeDumper();

        virtual void execute();

        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;
    };

}  // namespace WireCellApps

#endif
