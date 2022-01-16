// This adds noise to traces of its input frame to make a new output
// frame.  It should be given voltage-level input.  The model
// determins the noise waveform length so should be made to coincide
// with the length of input waveforms.  Thus, this component works
// only on rectangular, dense frames

#ifndef WIRECELL_GEN_ADDNOISE
#define WIRECELL_GEN_ADDNOISE

#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IChannelSpectrum.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellAux/Logger.h"

#include <string>

namespace WireCell {
    namespace Gen {

        class AddNoise : public Aux::Logger,
                         public IFrameFilter, public IConfigurable {
           public:
            AddNoise(const std::string& model = "", const std::string& rng = "Random");

            virtual ~AddNoise();

            /// IFrameFilter
            virtual bool operator()(const input_pointer& inframe, output_pointer& outframe);

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            IRandom::pointer m_rng;
            IChannelSpectrum::pointer m_model;

            std::string m_model_tn, m_rng_tn;
            int m_nsamples;
            double m_rep_percent;

            size_t m_count{0};
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
