/** A wrapper for pytorch torchscript using zio
 */

#ifndef WIRECELLPYTORCH_ZIOTORCHSCRIPT
#define WIRECELLPYTORCH_ZIOTORCHSCRIPT

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/ITensorSetFilter.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Pytorch {
        class ZioTorchScript : public ITensorSetFilter, public IConfigurable {
           public:
            ZioTorchScript();
            virtual ~ZioTorchScript() {}

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // ITensorSetFilter interface
            virtual bool operator()(const ITensorSet::pointer& in, ITensorSet::pointer& out);

           private:
            Log::logptr_t l;
            Configuration m_cfg;  /// copy of configuration

            std::unordered_map<std::string, float> m_timers;
        };
    }  // namespace Pytorch
}  // namespace WireCell

#endif  // WIRECELLPYTORCH_ZIOTORCHSCRIPT