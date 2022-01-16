#ifndef WIRECELL_IMPACTZIPPER
#define WIRECELL_IMPACTZIPPER

#include "WireCellIface/IPlaneImpactResponse.h"
#include "WireCellGen/BinnedDiffusion.h"

namespace WireCell {
    namespace Gen {

        /** An ImpactZipper "zips" up through all the impact positions
         * along a wire plane convolving the response functions and
         * the local drifted charge distribution producing a waveform
         * on each central wire.
         */
        class ImpactZipper {
            IPlaneImpactResponse::pointer m_pir;
            BinnedDiffusion& m_bd;

           public:
            ImpactZipper(IPlaneImpactResponse::pointer pir, BinnedDiffusion& bd);
            virtual ~ImpactZipper();

            /// Return the wire's waveform.  If the response functions
            /// are just field response (ie, instantaneous current)
            /// then the waveforms are expressed as current integrated
            /// over each sample bin and thus in units of charge.  If
            /// the response functions include electronics response
            /// then the waveforms are in units of voltage
            /// representing the sampling of the output of the FEE
            /// amplifiers.

            // fixme: this should be a forward iterator so that it may cal bd.erase() safely to conserve memory
            Waveform::realseq_t waveform(int wire) const;
        };

    }  // namespace Gen
}  // namespace WireCell
#endif /* WIRECELL_IMPACTZIPPER */
