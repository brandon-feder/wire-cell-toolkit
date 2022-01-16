#ifndef WIRECELL_AUX_TENSORSETUNPACKER
#define WIRECELL_AUX_TENSORSETUNPACKER

#include "WireCellIface/ITensorSetUnpacker.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Aux {

        // Fan out 1 frame to N set at construction or configuration time.
        class TensorSetUnpacker : public ITensorSetUnpacker, public IConfigurable {
           public:
            TensorSetUnpacker(size_t multiplicity = 0);
            virtual ~TensorSetUnpacker();

            // INode, override because we get multiplicity at run time.
            virtual std::vector<std::string> output_types();

            // IFanout
            virtual bool operator()(const input_pointer& in, output_vector& outv);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            size_t m_multiplicity;
            WireCell::Configuration m_cfg;
            Log::logptr_t log;
        };
    }  // namespace Aux
}  // namespace WireCell

#endif