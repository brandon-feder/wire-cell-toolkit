#ifndef WIRECELL_GEN_FRAMEFANIN
#define WIRECELL_GEN_FRAMEFANIN

#include "WireCellIface/IFrameFanin.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/TagRules.h"
#include "WireCellAux/Logger.h"

#include <vector>
#include <string>

namespace WireCell {
    namespace Gen {

        // Fan in N frames to one.
        class FrameFanin : Aux::Logger,
                           public IFrameFanin,
                           public IConfigurable
        {
          public:
            FrameFanin(size_t multiplicity = 2);
            virtual ~FrameFanin();

            virtual std::vector<std::string> input_types();

            virtual bool operator()(const input_vector& invec, output_pointer& out);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

          private:
            size_t m_multiplicity;
            std::vector<std::string> m_tags;
            tagrules::Context m_ft;
            int m_count{0};
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
