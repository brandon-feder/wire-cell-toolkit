#ifndef WIRECELLIMG_SLICESSINK
#define WIRECELLIMG_SLICESSINK

#include "WireCellIface/ISliceFrameSink.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Img {

        class SlicesSink : public ISliceFrameSink, public IConfigurable {
           public:
            SlicesSink();
            virtual ~SlicesSink();

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            bool operator()(const ISliceFrame::pointer& sf);

           private:
            Configuration m_cfg;
        };
    }  // namespace Img
}  // namespace WireCell

#endif
