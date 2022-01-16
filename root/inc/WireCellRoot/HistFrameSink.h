/** This is a frame sink that saves frames as TH2D histograms.

    It is configured with a filename.  If it contains a "%" character
    the filename will be formatted against the current frame ID.
 */

#ifndef WIRECELLROOT_HISTFRAMSINK
#define WIRECELLROOT_HISTFRAMSINK

#include "WireCellIface/IFrameSink.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"

#include <string>

namespace WireCell {
    namespace Root {

        class HistFrameSink : public IFrameSink, public IConfigurable {
           public:
            HistFrameSink();
            virtual ~HistFrameSink();

            /// Frame sink interface
            virtual bool operator()(const IFrame::pointer& frame);

            /// Configurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_filepat, m_anode_tn;
            IAnodePlane::pointer m_anode;
            double m_units;
        };

    }  // namespace Root
}  // namespace WireCell

#endif
