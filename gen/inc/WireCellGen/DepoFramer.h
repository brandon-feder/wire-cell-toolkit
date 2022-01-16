#ifndef WIRECELLGEN_DEPOFRAMER
#define WIRECELLGEN_DEPOFRAMER

#include "WireCellIface/IDepoFramer.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDrifter.h"
#include "WireCellIface/IDuctor.h"

namespace WireCell {
    namespace Gen {

        /** DepoFramer handles simulating the signal from a set of
         * depositions returning a frame of signal voltage traces.
         * The frame ident is taken from the the ident of the input
         * IDepoSet.  It delegates to a "drifter" and a "ductor" to
         * perform bulk drifting through the detector volume and
         * conversion of charge distribution to signals via
         * convolution with field and electronics response,
         * respectively.
         *
         * Because the output frame will span the entire signal
         * corresponding to the sent of input depositions, it is up to
         * the caller to limit this set accordingly.
         */
        class DepoFramer : public IDepoFramer, public IConfigurable {
           public:
            DepoFramer(const std::string& drifter = "Drifter", const std::string& ductor = "Ductor");
            virtual ~DepoFramer();

            // IDepoFramer interface
            virtual bool operator()(const input_pointer& in, output_pointer& out);

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_drifter_tn, m_ductor_tn;
            IDrifter::pointer m_drifter;
            IDuctor::pointer m_ductor;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
