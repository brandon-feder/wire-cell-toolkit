/** This is a frame sink that saves frames as celltree (TTree).

    It is configured with a filename.  If it contains a "%" character
    the filename will be formatted against the current frame ID.
 */

#ifndef WIRECELLROOT_CELLTREEFRAMSINK
#define WIRECELLROOT_CELLTREEFRAMSINK

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"

#include <string>

namespace WireCell {
    namespace Root {

        class CelltreeFrameSink : public IFrameFilter, public IConfigurable {
           public:
            CelltreeFrameSink();
            virtual ~CelltreeFrameSink();

            /// Frame sink interface
            virtual bool operator()(const IFrame::pointer& frame, IFrame::pointer& out_frame);

            /// Configurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            Configuration m_cfg;
            IAnodePlane::pointer m_anode;
            int m_nsamples;

            int m_nrebin;
        };

    }  // namespace Root
}  // namespace WireCell

#endif
