/** Init Decon2D for signal processing
 */

#ifndef WIRECELLSIG_DECON2DFILTER
#define WIRECELLSIG_DECON2DFILTER

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/ITensorSetFilter.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Sig {
        class Decon2DFilter : public ITensorSetFilter, public IConfigurable {
           public:
            Decon2DFilter();
            virtual ~Decon2DFilter() {}

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // ITensorSetFilter interface
            virtual bool operator()(const ITensorSet::pointer& in, ITensorSet::pointer& out);

           private:
            Log::logptr_t log;
            Configuration m_cfg;  /// copy of configuration
        };
    }  // namespace Sig
}  // namespace WireCell

#endif  // WIRECELLSIG_DECON2DFILTER