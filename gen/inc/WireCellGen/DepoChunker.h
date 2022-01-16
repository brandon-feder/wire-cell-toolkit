/* This collects depos over an advancing fixed window of time.  Each
 * time an input depo falls out of the window, a depo set is produced
 * and the window advances.  An input EOS flushes any accumulated
 * depos and resets the clock to the original starting time.
 *
 * This component is useful when one wants to simulate time
 * continuously but needs to chunk up the depos in order to, for
 * example, feed them to an IDepoFramer.
 *
 * This component is equivalent to the input action of the Ductor in
 * "continous" mode.
 */

#ifndef WIRECELLGEN_DEPOCHUNKER
#define WIRECELLGEN_DEPOCHUNKER

#include "WireCellIface/IDepoCollector.h"
#include "WireCellIface/IConfigurable.h"

#include <map>
#include <vector>

namespace WireCell {
    namespace Gen {

        class DepoChunker : public IDepoCollector, public IConfigurable {
           public:
            DepoChunker();
            virtual ~DepoChunker();

            // IDepoCollector
            virtual bool operator()(const input_pointer& depo, output_queue& deposetqueue);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            // Count how many we've produced, use this for the depo set ident.
            int m_count;

            // The current and starting acceptance time gates
            std::pair<double, double> m_gate, m_starting_gate;

            // Temporary holding of accepted depos.
            IDepo::vector m_depos;

            void emit(output_queue& out);
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
