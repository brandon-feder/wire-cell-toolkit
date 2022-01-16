/* FrameSplitter = FrameFanout + ChannelSelector (almost)

   This can be useful in order not to keep the input alive as each of
   the fan out branches are processed which otherwise defeats some
   lazy loading
 */

#ifndef WIRECELL_SIGPROC_CHANNELSPLITTER
#define WIRECELL_SIGPROC_CHANNELSPLITTER

#include "WireCellIface/IFrameFanout.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/TagRules.h"
#include "WireCellUtil/Logging.h"

#include <unordered_map>

namespace WireCell {
    namespace SigProc {

        class ChannelSplitter : public IFrameFanout, public IConfigurable {
           public:
            ChannelSplitter(size_t multiplicity = 0);
            virtual ~ChannelSplitter();

            // INode, override because we get multiplicity at run time.
            virtual std::vector<std::string> output_types();

            // IFanout
            virtual bool operator()(const input_pointer& in, output_vector& outv);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            // channel to port
            std::unordered_map<int, int> m_c2p;
            std::vector<std::string> m_tags;
            size_t m_multiplicity;
            tagrules::Context m_ft;
            Log::logptr_t log;
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
