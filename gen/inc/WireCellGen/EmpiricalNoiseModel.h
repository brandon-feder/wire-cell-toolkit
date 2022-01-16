/**
    A noise model based on empirically measured noise spectra.

    It requires configuration file holding a list of dictionary which
    provide association between wire length and the noise spectrum.

    TBD: document JSON file format for providing spectra and any other parameters.

*/

#ifndef WIRECELLGEN_EMPERICALNOISEMODEL
#define WIRECELLGEN_EMPERICALNOISEMODEL

#include "WireCellIface/IChannelSpectrum.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IChannelStatus.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellAux/Logger.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace WireCell {
    namespace Gen {

        class EmpiricalNoiseModel : public Aux::Logger,
                                    public IChannelSpectrum, public IConfigurable {
           public:
            EmpiricalNoiseModel(const std::string& spectra_file = "",
                                const int nsamples = 10000,  // assuming 10k samples
                                const double period = 0.5 * units::us, const double wire_length_scale = 1.0 * units::cm,
                                /* const double time_scale = 1.0*units::ns, */
                                /* const double gain_scale = 1.0*units::volt/units::eplus, */
                                /* const double freq_scale = 1.0*units::megahertz, */
                                const std::string anode_tn = "AnodePlane",
                                const std::string chanstat_tn = "StaticChannelStatus");

            virtual ~EmpiricalNoiseModel();

            /// IChannelSpectrum
            virtual const amplitude_t& operator()(int chid) const;

            // get constant term
            virtual const std::vector<float>& freq() const;
            // get json file gain
            virtual const double gain(int chid) const;
            // get json file shaping time
            virtual const double shaping_time(int chid) const;

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            // Local methods

            // fixme: this should be factored out into wire-cell-util.
            struct NoiseSpectrum {
                int plane;                 // plane identifier number
                int nsamples;              // number of samples used in preparing spectrum
                double period;             // sample period [time] used in preparing spectrum
                double gain;               // amplifier gain [voltage/charge]
                double shaping;            // amplifier shaping time [time]
                double wirelen;            // total length of wire conductor [length]
                double constant;           // amplifier independent constant noise component [voltage/frequency]
                std::vector<float> freqs;  // the frequencies at which the spectrum is sampled
                std::vector<float> amps;   // the amplitude [voltage/frequency] of the spectrum.
            };

            // Resample a NoiseSpectrum to match what the model was
            // configured to provide.  This method modifies in place.
            void resample(NoiseSpectrum& spectrum) const;

            // generate the default electronics response function at 1 mV/fC gain
            void gen_elec_resp_default();

            // Return a new amplitude which is the interpolation
            // between those given in the spectra file.
            amplitude_t interpolate(int plane, double wire_length) const;

            int get_nsamples() { return m_nsamples; };

           private:
            IAnodePlane::pointer m_anode;
            IChannelStatus::pointer m_chanstat;

            std::string m_spectra_file;
            int m_nsamples;
            int m_fft_length;
            double m_period, m_wlres;
            // double m_tres, m_gres, m_fres;
            std::string m_anode_tn, m_chanstat_tn;

            std::map<int, std::vector<NoiseSpectrum*> > m_spectral_data;

            // cache amplitudes to the nearest integral distance.
            mutable std::unordered_map<int, int> m_chid_to_intlen;

            typedef std::unordered_map<int, amplitude_t> len_amp_cache_t;
            mutable std::vector<len_amp_cache_t> m_amp_cache;

            // need to convert the electronics response in here ...
            Waveform::realseq_t m_elec_resp_freq;
            mutable std::unordered_map<int, Waveform::realseq_t> m_elec_resp_cache;
            mutable amplitude_t comb_amp;

        };

    }  // namespace Gen
}  // namespace WireCell

#endif
