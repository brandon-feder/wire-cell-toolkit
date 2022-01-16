/** Read in IFrame in hdf5 format
 */

#ifndef WIRECELLHDF5_HDF5FRAMESOURCE
#define WIRECELLHDF5_HDF5FRAMESOURCE

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Hio {

        class HDF5FrameSource : public IFrameSource, public IConfigurable {
           public:
            HDF5FrameSource();
            virtual ~HDF5FrameSource();

            /// IFrameSource
            virtual bool operator()(IFrame::pointer &out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration &config);

           private:
            Configuration m_cfg;           /// copy of configuration
            IAnodePlane::pointer m_anode;  /// pointer to some APA, needed to associate chnnel ID to planes

            std::vector<std::string> m_filenames;
            std::string m_policy;
            Log::logptr_t l;

            IFrame::vector m_frames;  // current set of depos
        };
    }  // namespace Hio
}  // namespace WireCell

#endif