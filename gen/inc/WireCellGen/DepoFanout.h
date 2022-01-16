#ifndef WIRECELL_GEN_DEPOFANOUT
#define WIRECELL_GEN_DEPOFANOUT

#include "WireCellIface/IDepoFanout.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Gen {

        // Fan out 1 depo to N set at construction or configuration time.
        class DepoFanout : public Aux::Logger,
                           public IDepoFanout, public IConfigurable {
           public:
            DepoFanout(size_t multiplicity = 0);
            virtual ~DepoFanout();

            // INode, override because we get multiplicity at run time.
            virtual std::vector<std::string> output_types();

            // IFanout
            virtual bool operator()(const input_pointer& in, output_vector& outv);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            size_t m_multiplicity;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
