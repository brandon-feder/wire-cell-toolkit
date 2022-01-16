/** This frame source provides frames filled with noise.

    Each time it is called it produces a fixed readout length of
    voltage-level noise which spans all channels.

 */

#ifndef WIRECELLGEN_NOISESOURCE
#define WIRECELLGEN_NOISESOURCE

#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IChannelSpectrum.h"
#include "WireCellUtil/Waveform.h"

#include <string>

namespace WireCell {
    namespace Gen {

        class NoiseSource : public IFrameSource, public IConfigurable {
           public:
            NoiseSource(const std::string& model = "", const std::string& anode = "AnodePlane",
                        const std::string& rng = "Random");
            // fixme: add constructor that set parameter defaults from c++ for unit tests
            virtual ~NoiseSource();

            /// IFrameSource
            virtual bool operator()(IFrame::pointer& frame);

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            IRandom::pointer m_rng;
            IAnodePlane::pointer m_anode;
            IChannelSpectrum::pointer m_model;
            double m_time, m_stop, m_readout, m_tick;
            int m_frame_count;
            std::string m_anode_tn, m_model_tn, m_rng_tn;
            int m_nsamples;
            double m_rep_percent;
            bool m_eos;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
