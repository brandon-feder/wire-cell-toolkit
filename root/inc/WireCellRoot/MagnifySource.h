/** A "Magnify" file is one used for viewing in the Magnify display.
 *
 * The file can only hold one frame so this source may be called at
 * most twice before it starts returning false.  First time will give
 * a frame, second time will give an EOS.
 */

#ifndef WIRECELLROOT_MAGNIFYFILESOURCE
#define WIRECELLROOT_MAGNIFYFILESOURCE

#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Root {

        class MagnifySource : public IFrameSource, public IConfigurable {
           public:
            MagnifySource();
            virtual ~MagnifySource();

            /// IFrameSource
            virtual bool operator()(IFrame::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

           private:
            Configuration m_cfg;
            int m_calls;
        };
    }  // namespace Root
}  // namespace WireCell

#endif
