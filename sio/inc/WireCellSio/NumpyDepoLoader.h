/** Load depos from a Numpy file. 

    Depos should consist of arrays named depo_data_N and depo_info_N
    where N is an integer count.

    For a given value of N the data array should be shaped (Ndepo, 7),
    and info is shaped (Ndepo, 4).

    See also @ref NumpyDepoSaver.

*/

#ifndef WIRECELLSIO_NUMPYDEPOLOADER
#define WIRECELLSIO_NUMPYDEPOLOADER

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

#include <deque>

namespace WireCell {
    namespace Sio {

        // This loader is the counterpart to NumpyDepoSaver.
        class NumpyDepoLoader : public WireCell::Aux::Logger,
                                public WireCell::IDepoSource,
                                public WireCell::IConfigurable {
          public:
            NumpyDepoLoader();
            virtual ~NumpyDepoLoader();

            /// IDepoSource.  This faithfully reproduces the EOS
            /// behavior followed by NumpyDepoSaver when it wrote the
            /// file.
            virtual bool operator()(IDepo::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

          protected:
            // try to load next array
            bool next();

            Configuration m_cfg;
            int m_load_count;  // count frames loaded
            int m_eos;         // keep track if outside a stream

            // buffer one frame of depos
            std::deque<WireCell::IDepo::pointer> m_depos;
        };

    }  // namespace Sio
}  // namespace WireCell
#endif
