/** MultiDuctor - apply one of many possible ductors based on outcome of rules applied to input depos.

    A list of independent "chains" are given.  Each chain is applied
    to an input depo.  A chain is a sequence of rules.  Each rule is
    applied in turn to a depo until one matches.  On a match, the
    ductor associated with the rule is given the depo and subsequent
    iteration of the chain is abandoned.
 */

#ifndef WIRECELLGEN_MULTIDUCTOR
#define WIRECELLGEN_MULTIDUCTOR

#include "WireCellIface/IDuctor.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"

#include <functional>

namespace WireCell {
    namespace Gen {

        class MultiDuctor : public IDuctor, public IConfigurable {
           public:
            MultiDuctor(const std::string anode = "AnodePlane");
            virtual ~MultiDuctor();

            // virtual void reset();
            // IDuctor
            virtual bool operator()(const input_pointer& depo, output_queue& frames);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_anode_tn;
            IAnodePlane::pointer m_anode;
            double m_tick;
            double m_start_time;
            double m_readout_time;
            int m_frame_count;
            bool m_continuous;
            bool m_eos;

            struct SubDuctor {
                std::string name;
                std::function<bool(IDepo::pointer depo)> check;
                IDuctor::pointer ductor;
                SubDuctor(const std::string& tn, std::function<bool(IDepo::pointer depo)> f, IDuctor::pointer d)
                  : name(tn)
                  , check(f)
                  , ductor(d)
                {
                }
            };
            typedef std::vector<SubDuctor> ductorchain_t;
            std::vector<ductorchain_t> m_chains;

            /// As sub ductors are called they will each return frames
            /// which are not in general synchronized with the others.
            /// Their frames must be buffered here and released as a
            /// merged frame in order for MultiDuctor to behave just
            /// like a monolithic ductor.
            output_queue m_frame_buffer;

            // local

            // Accept new frames into the buffer
            void merge(const output_queue& newframes);

            // Maybe extract output frames from the buffer.  If the
            // depo is past the next scheduled readout or if a nullptr
            // depo (EOS) then sub-ductors are flushed with EOS and
            // the outframes queue is filled with one or more frames.
            // If extraction occurs not by EOS then any carryover is
            // kept.
            void maybe_extract(const input_pointer& depo, output_queue& outframes);

            // Return true if depo indicates it is time to start
            // processing.  Will set start time if in continuous mode.
            bool start_processing(const input_pointer& depo);

            void dump_frame(const IFrame::pointer frame, std::string msg = "Gen::MultiDuctor:");
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
