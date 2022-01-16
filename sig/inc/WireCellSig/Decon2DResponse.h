/** Init Decon2D for signal processing
 */

#ifndef WIRECELLSIG_DECON2DRESPONSE
#define WIRECELLSIG_DECON2DRESPONSE

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/ITensorSetFilter.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IChannelResponse.h"
#include "WireCellIface/IFieldResponse.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Sig {
        class Decon2DResponse : public ITensorSetFilter, public IConfigurable {
           public:
            Decon2DResponse();
            virtual ~Decon2DResponse() {}

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // ITensorSetFilter interface
            virtual bool operator()(const ITensorSet::pointer& in, ITensorSet::pointer& out);

           private:
            std::vector<Waveform::realseq_t> init_overall_response(const ITensorSet::pointer& in) const;

            Log::logptr_t log;
            Configuration m_cfg;  /// copy of configuration

            // pointer to IAnodePlane
            IAnodePlane::pointer m_anode;

            IChannelResponse::pointer m_cresp;

            IFieldResponse::pointer m_fresp;
        };
    }  // namespace Sig
}  // namespace WireCell

#endif  // WIRECELLSIG_DECON2DRESPONSE