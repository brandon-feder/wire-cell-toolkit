/** Inherently protoDUNE-specific functions and classes
 *  Modified from Microboone.h
 */

#ifndef WIRECELLSIGPROC_PROTODUNE
#define WIRECELLSIGPROC_PROTODUNE

#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Bits.h"
#include "WireCellIface/IChannelFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IChannelNoiseDatabase.h"
#include "WireCellIface/IAnodePlane.h"

#include "WireCellSigProc/Diagnostics.h"

namespace WireCell {
    namespace SigProc {
        namespace Protodune {

            bool LinearInterpSticky(WireCell::Waveform::realseq_t& signal, std::vector<std::pair<int, int> >& st_ranges,
                                    float stky_sig_like_val, float stky_sig_like_rms);
            bool FftInterpSticky(WireCell::Waveform::realseq_t& signal, std::vector<std::pair<int, int> >& st_ranges);
            bool FftShiftSticky(WireCell::Waveform::realseq_t& signal, double toffset,
                                std::vector<std::pair<int, int> >& st_ranges);
            bool FftScaling(WireCell::Waveform::realseq_t& signal, int nsamples);

            // hold common config stuff
            class ConfigFilterBase : public WireCell::IConfigurable {
               public:
                ConfigFilterBase(const std::string& anode = "AnodePlane",
                                 const std::string& noisedb = "OmniChannelNoiseDB");
                virtual ~ConfigFilterBase();

                // IConfigurable configuration interface
                virtual void configure(const WireCell::Configuration& config);
                virtual WireCell::Configuration default_configuration() const;

                // FIXME: this method needs to die.
                void set_channel_noisedb(WireCell::IChannelNoiseDatabase::pointer ndb) { m_noisedb = ndb; }

               protected:
                std::string m_anode_tn, m_noisedb_tn;
                IAnodePlane::pointer m_anode;
                IChannelNoiseDatabase::pointer m_noisedb;
            };

            /** Microboone/ProtoDUNE style noise subtraction.
             *
             * Fixme: in principle, this class could be general purpose
             * for other detectors.  However, it uses the functions above
             * which hard code microboone-isms.  If those
             * microboone/protodune-specific parameters can be pulled out to a
             * higher layer then this class can become generic and move
             * outside of this file.
             */

            class StickyCodeMitig : public WireCell::IChannelFilter, public WireCell::IConfigurable {
               public:
                StickyCodeMitig(const std::string& anode_tn = "AnodePlane",
                                const std::string& noisedb = "OmniChannelNoiseDB", float stky_sig_like_val = 15.0,
                                float stky_sig_like_rms = 2.0, int stky_max_len = 5);
                virtual ~StickyCodeMitig();

                // IChannelFilter interface

                /** Filter in place the signal `sig` from given `channel`. */
                virtual WireCell::Waveform::ChannelMaskMap apply(int channel, signal_t& sig) const;

                /** Filter in place a group of signals together. */
                virtual WireCell::Waveform::ChannelMaskMap apply(channel_signals_t& chansig) const;

                virtual void configure(const WireCell::Configuration& config);
                virtual WireCell::Configuration default_configuration() const;

               private:
                std::string m_anode_tn, m_noisedb_tn;
                IAnodePlane::pointer m_anode;
                IChannelNoiseDatabase::pointer m_noisedb;

                std::map<int, std::vector<short int> > m_extra_stky;  // ch to extra sticky codes
                float m_stky_sig_like_val;
                float m_stky_sig_like_rms;
                int m_stky_max_len;
            };

            class OneChannelNoise : public WireCell::IChannelFilter, public ConfigFilterBase {
               public:
                OneChannelNoise(const std::string& anode_tn = "AnodePlane",
                                const std::string& noisedb = "OmniChannelNoiseDB");
                virtual ~OneChannelNoise();

                // IChannelFilter interface

                /** Filter in place the signal `sig` from given `channel`. */
                virtual WireCell::Waveform::ChannelMaskMap apply(int channel, signal_t& sig) const;

                /** Filter in place a group of signals together. */
                virtual WireCell::Waveform::ChannelMaskMap apply(channel_signals_t& chansig) const;

                void configure(const WireCell::Configuration& config);
                WireCell::Configuration default_configuration() const;

               private:
                Diagnostics::Partial m_check_partial;  // at least need to expose them to configuration
                std::map<int, int> m_resmp;            // ch => orignal smp input
            };

            // A relative gain correction based on David Adam's pulse area calibration
            class RelGainCalib : public WireCell::IChannelFilter, public WireCell::IConfigurable {
               public:
                RelGainCalib(const std::string& anode_tn = "AnodePlane",
                             const std::string& noisedb = "OmniChannelNoiseDB", float gain_def = 1.0,
                             float gain_min_cut = 0.8, float gain_max_cut = 1.25);
                virtual ~RelGainCalib();

                // IChannelFilter interface

                /** Filter in place the signal `sig` from given `channel`. */
                virtual WireCell::Waveform::ChannelMaskMap apply(int channel, signal_t& sig) const;

                /** Filter in place a group of signals together. */
                virtual WireCell::Waveform::ChannelMaskMap apply(channel_signals_t& chansig) const;

                virtual void configure(const WireCell::Configuration& config);
                virtual WireCell::Configuration default_configuration() const;

               private:
                std::string m_anode_tn, m_noisedb_tn;
                IAnodePlane::pointer m_anode;
                IChannelNoiseDatabase::pointer m_noisedb;

                float m_gain_def;
                float m_gain_min_cut;
                float m_gain_max_cut;

                std::vector<float> m_rel_gain;  // relative gain map
            };

        }  // namespace Protodune

    }  // namespace SigProc

}  // namespace WireCell

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
