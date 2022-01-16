#ifndef WIRECELL_GEN_DEPOSETFANOUT
#define WIRECELL_GEN_DEPOSETFANOUT

#include "WireCellIface/IDepoSetFanout.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Gen {

        // Fan out 1 deposet to N set at construction or configuration time.
        class DepoSetFanout : public Aux::Logger,
                              public IDepoSetFanout, public IConfigurable {
           public:
            DepoSetFanout(size_t multiplicity = 0);
            virtual ~DepoSetFanout();

            // INode, override because we get multiplicity at run time.
            virtual std::vector<std::string> output_types();

            // IFanout
            virtual bool operator()(const input_pointer& in, output_vector& outv);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            size_t m_multiplicity;
            size_t m_count{0};

            
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
