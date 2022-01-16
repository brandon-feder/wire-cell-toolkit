#ifndef WIRECELLGEN_DUCTOR
#define WIRECELLGEN_DUCTOR

#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Response.h"

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDuctor.h"

#include "WireCellIface/IAnodeFace.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IPlaneImpactResponse.h"
#include "WireCellIface/IRandom.h"
#include "WireCellUtil/Logging.h"

#include <vector>

namespace WireCell {
    namespace Gen {

        /** This IDuctor needs a Garfield2D field calculation data
         * file in compressed JSON format as produced by Python module
         * wirecell.sigproc.garfield.
         */
        class Ductor : public IDuctor, public IConfigurable {
           public:
            Ductor();
            virtual ~Ductor(){};

            // virtual void reset();
            virtual bool operator()(const input_pointer& depo, output_queue& frames);

            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           protected:
            // The "Type:Name" of the IAnodePlane (default is "AnodePlane")
            std::string m_anode_tn;
            std::string m_rng_tn;
            std::vector<std::string> m_pir_tns;

            IAnodePlane::pointer m_anode;
            IRandom::pointer m_rng;
            std::vector<IPlaneImpactResponse::pointer> m_pirs;

            IDepo::vector m_depos;

            double m_start_time;
            double m_readout_time;
            double m_tick;
            double m_drift_speed;
            double m_nsigma;
            bool m_fluctuate;
            std::string m_mode;

            int m_frame_count;
            std::string m_tag;

            virtual void process(output_queue& frames);
            virtual ITrace::vector process_face(IAnodeFace::pointer face, const IDepo::vector& face_depos);
            bool start_processing(const input_pointer& depo);
            Log::logptr_t l;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
