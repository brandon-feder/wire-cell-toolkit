/** Sink data to a file format used by the "Magnify" GUI .
 *
 * This is technically a "filter" as it passes on its input.  This
 * allows an instance of the sink to sit in the middle of some longer
 * chain.
 *
 * FIXME: currently this class TOTALLY violates the encapsulation of
 * DFP by requiring the input file in order to transfer input data out
 * of band of the flow.
 */

#ifndef WIRECELLHDF5_HDF5FRAMETAP
#define WIRECELLHDF5_HDF5FRAMETAP

#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFrameSink.h"
#include "WireCellIface/IFrameFilter.h"
#include "WireCellUtil/Logging.h"

namespace WireCell {
    namespace Hio {

        class HDF5FrameTap : public IFrameFilter, public IConfigurable {
           public:
            HDF5FrameTap();
            virtual ~HDF5FrameTap();

            /// working operation - interface from IFrameFilter
            /// executed when called by pgrapher
            virtual bool operator()(const IFrame::pointer &inframe, IFrame::pointer &outframe);

            /// interfaces from IConfigurable

            /// exeexecuted once at node creation
            virtual WireCell::Configuration default_configuration() const;

            /// executed once after node creation
            virtual void configure(const WireCell::Configuration &config);

           private:
            Configuration m_cfg;           /// copy of configuration
            IAnodePlane::pointer m_anode;  /// pointer to some APA, needed to associate chnnel ID to planes

            int m_save_count;  // count frames saved

            /// SPD logger
            Log::logptr_t l;
        };
    }  // namespace Hio
}  // namespace WireCell

#endif
