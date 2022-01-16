/* This collects depos inside a fixed time gate and releases the
 * collection on an input EOS.
 *
 * This component is useful, for example, in the case of depos
 * produced by uB LArG4 into art::Event where each "event" is near
 * t=0.0.
 *
 * This component is equivalent to the input action of the Ductor in
 * "fixed" mode.
 *
 * If the gate is kept as default [0,0] then all depos are bagged.
 */

#ifndef WIRECELLGEN_DEPOBAGGER
#define WIRECELLGEN_DEPOBAGGER

#include "WireCellIface/IDepoCollector.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

#include <map>
#include <vector>

namespace WireCell {
    namespace Gen {

        class DepoBagger : public Aux::Logger,
                           public IDepoCollector, public IConfigurable {
           public:
            DepoBagger();
            virtual ~DepoBagger();

            // IDepoCollector
            virtual bool operator()(const input_pointer& depo, output_queue& deposetqueue);

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            // Count how many we've produced, use this for the depo set ident.
            int m_count{0};

            // The acceptance time gate
            std::pair<double, double> m_gate{0,0};

            // Temporary holding of accepted depos.
            IDepo::vector m_depos;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
