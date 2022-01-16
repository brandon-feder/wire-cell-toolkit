/** Save depos to a Numpy file. */

#ifndef WIRECELLSIO_NUMPYDEPOSAVER
#define WIRECELLSIO_NUMPYDEPOSAVER

#include "WireCellIface/IDepoFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Sio {

        // This saver will buffer depos in memory until EOS is received.
        class NumpyDepoSaver : public Aux::Logger, public WireCell::IDepoFilter, public WireCell::IConfigurable {
          public:
            NumpyDepoSaver();
            virtual ~NumpyDepoSaver();

            /// IDepoFilter.  This works by buffering depos and saving
            /// them at the same time a frame is saved.
            virtual bool operator()(const WireCell::IDepo::pointer& indepo, WireCell::IDepo::pointer& outdepo);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

          private:
            std::string m_filename{"wct-depos.npz"};
            int m_save_count{0}; 
            std::vector<WireCell::IDepo::pointer> m_depos;
        };

    }  // namespace Sio
}  // namespace WireCell
#endif
