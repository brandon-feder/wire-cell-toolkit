#ifndef WIRECELLAPPS_CONFIGDUMPER
#define WIRECELLAPPS_CONFIGDUMPER

#include "WireCellIface/IApplication.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Configuration.h"

namespace WireCellApps {

    class ConfigDumper : public WireCell::IApplication, public WireCell::IConfigurable {
        WireCell::Configuration m_cfg;

       public:
        ConfigDumper();
        virtual ~ConfigDumper();

        virtual void execute();

        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;
    };

}  // namespace WireCellApps

#endif
