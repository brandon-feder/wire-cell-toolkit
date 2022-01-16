#ifndef WIRECELLGEN_TRUTHSMEARER
#define WIRECELLGEN_TRUTHSMEARER

#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Response.h"

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDuctor.h"

#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IRandom.h"

namespace WireCell {
    namespace Gen {

        class TruthSmearer : public IDuctor, public IConfigurable {
           public:
            TruthSmearer();

            // virtual void reset();
            virtual bool operator()(const input_pointer& depo, output_queue& frames);

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            // The "Type:Name" of the IAnodePlane (default is "AnodePlane")
            std::string m_anode_tn;
            std::string m_rng_tn;

            IAnodePlane::pointer m_anode;
            IRandom::pointer m_rng;
            IDepo::vector m_depos;

            double m_start_time;
            double m_readout_time;
            double m_tick;
            double m_drift_speed;

            double m_time_smear;
            double m_wire_smear_ind;
            double m_wire_smear_col;
            std::string m_smear_response_tn;
            double m_truth_gain;  // sign of charge depos in output

            double m_nsigma;
            bool m_fluctuate;
            bool m_continuous;

            int m_frame_count;

            void process(output_queue& frames);
            bool start_processing(const input_pointer& depo);
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
