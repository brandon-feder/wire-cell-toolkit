/** This is Omnibus, a WCT app object that ties together signal
 * processing components.  It takes any number of frame filters and
 * pushes one frame from source, through the pipeline to sink. */

#ifndef WIRECELLSIGPROC_OMNIBUS
#define WIRECELLSIGPROC_OMNIBUS

#include "WireCellIface/IApplication.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IFrameSink.h"

#include <vector>
#include <string>

namespace WireCell {
    namespace SigProc {

        class Omnibus : public WireCell::IApplication, public WireCell::IConfigurable {
           public:
            Omnibus();
            virtual ~Omnibus();

            virtual void execute();

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_input_tn, m_output_tn;
            std::vector<std::string> m_filters_tn;
            IFrameSource::pointer m_input;
            std::vector<IFrameFilter::pointer> m_filters;
            IFrameSink::pointer m_output;
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
