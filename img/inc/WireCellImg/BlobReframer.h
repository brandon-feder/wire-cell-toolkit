/* A blob reframer outputs a frame for each input cluster of blobs.
 *
 * The output frame will only have samples which contribute to channel
 * and time regions covered by the blobs.  The output samples are
 * taken from the frame that is reached back via blob->slice->frame.
 * If multiple frames are reached they are nonetheless combined into
 * one output frame.
 */

#ifndef WIRECELLIMG_BLOBREFRAMER
#define WIRECELLIMG_BLOBREFRAMER

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IClusterFramer.h"
#include "WireCellAux/Logger.h"

namespace WireCell {

    namespace Img {

        class BlobReframer : public Aux::Logger, public IClusterFramer, public IConfigurable {
           public:
            BlobReframer(const std::string& frame_tag = "reframe");
            virtual ~BlobReframer();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& in, output_pointer& out);

           private:
            int m_nticks;
            double m_period;
            std::string m_frame_tag;

        };
    }  // namespace Img
}  // namespace WireCell

#endif
