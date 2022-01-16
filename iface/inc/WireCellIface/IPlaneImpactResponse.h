/** A plane impact response provides information about a collection of
 * impact responses for a plane.  An impact response is a complex
 * Fourier spectrum of some response (eg, field and possibly also
 * electronics) to a point charge on a drift path that starts at the
 * given impact position.  Impact positions provide a regular
 * subdivision along the pitch direction for a wire plane.  Typically
 * ten impact positions per wire region are used and with one impact
 * position exactly aligned to a wire position.
 */

#ifndef WIRECELL_IFACE_IPLANEIMPACTRESPONSE
#define WIRECELL_IFACE_IPLANEIMPACTRESPONSE

#include "WireCellUtil/IComponent.h"
#include "WireCellIface/IAnodeFace.h"
#include "WireCellUtil/Waveform.h"

namespace WireCell {

    class IImpactResponse : public IComponent<IImpactResponse> {
       public:
        virtual ~IImpactResponse();

        /// Frequency-domain spectrum of response
        virtual const Waveform::compseq_t& spectrum() const = 0;
        /// Time-domain waveform of the response
        virtual const Waveform::realseq_t& waveform() const = 0;
        // minimum padding for the response
        virtual int waveform_pad() const = 0;

        // Time-domain waveform of the auxillary long-range response
        virtual const Waveform::realseq_t& long_aux_waveform() const = 0;
        // minimum padding for the auxillary long-range response
        virtual int long_aux_waveform_pad() const = 0;

        /// Corresponding impact number
        virtual int impact() const = 0;
    };

    // A pair of IRs
    typedef std::pair<IImpactResponse::pointer, IImpactResponse::pointer> TwoImpactResponses;

    class IPlaneImpactResponse : public IComponent<IPlaneImpactResponse> {
       public:
        virtual ~IPlaneImpactResponse();

        /// Return the response at the impact position closest to
        /// the given relative pitch location (measured relative
        /// to the wire of interest).
        virtual IImpactResponse::pointer closest(double relpitch) const = 0;

        /// Return the two responses which are associated with the
        /// impact positions on either side of the given pitch
        /// location (measured relative to the wire of interest).
        virtual TwoImpactResponses bounded(double relpitch) const = 0;

        /// The extent in pitch covered by the impact positions.  This
        /// extent is relative to and centered on the "wire of
        /// interest" for which the PIR is associated.
        virtual double pitch_range() const = 0;

        /// The number of wires that span the pitch range.
        virtual int nwires() const = 0;

        // /// Return the distance between wires along the wire pitch direction.
        virtual double pitch() const = 0;

        // /// Return the distance between impact positions along the
        // /// wire pitch direction.
        virtual double impact() const = 0;

        // /// Return the number of bins in the impact response spectra.
        virtual size_t nbins() const = 0;
    };
}  // namespace WireCell

#endif
