#ifndef WIRECELL_GEN_FRAMEFANOUT
#define WIRECELL_GEN_FRAMEFANOUT

#include "WireCellIface/IFrameFanout.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/TagRules.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Gen {

        /// Fan out 1 frame to N set at construction or configuration time.
        ///
        /// If given no rules it works in a trivial manner to simply
        /// forward the input frame to its outputs.  If rules are
        /// given then the fanout applies tag filtering and rewriting.
        class FrameFanout : public Aux::Logger,
                            public IFrameFanout, public IConfigurable {
           public:
            FrameFanout(size_t multiplicity = 0);
            virtual ~FrameFanout();

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

            bool m_trivial{false};
            tagrules::Context m_ft;

        };
    }  // namespace Gen
}  // namespace WireCell

#endif
