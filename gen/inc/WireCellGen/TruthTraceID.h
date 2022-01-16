#ifndef WIRECELLGEN_TRUTHTRACEID
#define WIRECELLGEN_TRUTHTRACEID

#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/Binning.h"

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDuctor.h"

#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IRandom.h"

namespace WireCell {
    namespace Gen {
        class TruthTraceID : public IDuctor, public IConfigurable {
           public:
            TruthTraceID();

            virtual void reset();
            virtual bool operator()(const input_pointer& depo, output_queue& frames);
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_anode_tn;
            std::string m_rng_tn;

            IAnodePlane::pointer m_anode;
            IRandom::pointer m_rng;
            IDepo::vector m_depos;

            double m_start_time;
            double m_readout_time;
            double m_tick, m_pitch_range;
            double m_drift_speed;
            double m_nsigma;
            double m_truth_gain;
            bool m_fluctuate;

            int m_frame_count;
            bool m_eos;

            std::string m_truth_type;
            double m_num_ind_wire;
            double m_num_col_wire;
            double m_ind_sigma;
            double m_col_sigma;
            double m_time_sigma;
            double m_wire_power;
            double m_time_power;
            double m_max_wire_freq;
            double m_max_time_freq;
            bool m_wire_flag;
            bool m_time_flag;

            void process(output_queue& frame);
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
