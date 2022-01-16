#ifndef WIRECELL_AUX_TENSORPACKER
#define WIRECELL_AUX_TENSORPACKER

#include "WireCellIface/ITensorPacker.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Aux {

        // Fan out 1 frame to N set at construction or configuration time.
        class TensorPacker : public ITensorPacker, public IConfigurable {
           public:
            TensorPacker(size_t multiplicity = 0);
            virtual ~TensorPacker();

            // INode, override because we get multiplicity at run time.
            virtual std::vector<std::string> input_types();

            // IFanout
            virtual bool operator()(const input_vector& invec, output_pointer& out);

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