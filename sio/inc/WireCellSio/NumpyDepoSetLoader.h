/** Load depo sets from a Numpy file. 

    Depos should consist of arrays named depo_data_N and depo_info_N
    where N is an integer count.

    For a given value of N the data array should be shaped (Ndepo, 7),
    and info is shaped (Ndepo, 4).

    See also @ref NumpyDepoSaver.

*/

#ifndef WIRECELLSIO_NUMPYDEPOSETLOADER
#define WIRECELLSIO_NUMPYDEPOSETLOADER

#include "WireCellIface/IDepoSetSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

#include <deque>

namespace WireCell {
    namespace Sio {

        // This loader is the counterpart to NumpyDepoSaver.
        class NumpyDepoSetLoader : public WireCell::Aux::Logger,
                                   public WireCell::IDepoSetSource,
                                   public WireCell::IConfigurable
        {
          public:
            NumpyDepoSetLoader();
            virtual ~NumpyDepoSetLoader();

            /// IDepoSetSource.  
            virtual bool operator()(IDepoSet::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

          private:

            Configuration m_cfg;
            int m_load_count{0}; // count frames loaded
            bool m_sent_eos{false};
        };

    }  // namespace Sio
}  // namespace WireCell
#endif


