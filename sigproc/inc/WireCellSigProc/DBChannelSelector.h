/**
 Database Channel Selector:
 select channels from database
 */

#ifndef WIRECELLSIGPROC_DBCHANNELSELECTOR
#define WIRECELLSIGPROC_DBCHANNELSELECTOR

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IChannelNoiseDatabase.h"
#include "WireCellSigProc/ChannelSelector.h"

#include <string>
#include <vector>

namespace WireCell {
    namespace SigProc {

        class DBChannelSelector : public ChannelSelector {
           public:
            DBChannelSelector();
            virtual ~DBChannelSelector();

            /// IFrameFilter interface.
            bool operator()(const input_pointer& in, output_pointer& out);

            /// IConfigurable interface.
            void configure(const WireCell::Configuration& config);
            WireCell::Configuration default_configuration() const;

           private:
            std::string m_type;
            WireCell::IChannelNoiseDatabase::pointer m_db;
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
