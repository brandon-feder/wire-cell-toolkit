/**
   A Channel Spectrum provides a discrete Fourier amplitude as a
   function of frequency.

   An example implementation is a model of the noise on a given
   channel.

 */

#ifndef WIRECELL_ICHANNELSPECTRUM
#define WIRECELL_ICHANNELSPECTRUM

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class IChannelSpectrum : virtual public IComponent<IChannelSpectrum> {
       public:
        virtual ~IChannelSpectrum();

        /// The data type for frequency space amplitude (not power).
        /// It should be in units of [X]/[frequency] (equivalently
        /// [X]*[time]) where [X] is the unit of the equivalent
        /// waveform expressed in the time domain.  For a noise model
        /// [X] is likely [voltage].
        typedef std::vector<float> amplitude_t;

        /// Return the spectrum associated with the given channel ID
        /// in suitable binning.  In the implementing component, the
        /// Binning should likely be coordinated with the rest of the
        /// application via the configuration.
        virtual const amplitude_t& operator()(int chid) const = 0;
    };
}  // namespace WireCell

#endif
