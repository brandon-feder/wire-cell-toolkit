/*
  This component will provide depositions from a JSON file.

  The JSON file schema is assumed to be a dictionary with a list of
  depositions found by key "depos" (by default).  Each entry in the
  list is a dictionary with keys:

  - x,y,z :: the Cartesian spatial coordinates in cm. (fixme: really?)
  - t : time in seconds (fixme: really?)
  - q : point energy deposition in MeV
or
  - q,s : energy deposition q in MeV along step of length s in cm (fixme: really?)
  - n : number of electrons.


 */

#ifndef WIRECELLSIO_JSONDEPOSOURCE
#define WIRECELLSIO_JSONDEPOSOURCE

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Sio {

        class JsonRecombinationAdaptor;
        class JsonDepoSource : public IDepoSource, public IConfigurable {
           public:
            JsonDepoSource();
            virtual ~JsonDepoSource();

            /// IDepoSource
            virtual bool operator()(IDepo::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

            // local helper method
            IDepo::pointer jdepo2idepo(Json::Value jdepo);

           private:
            JsonRecombinationAdaptor* m_adapter;
            WireCell::IDepo::vector m_depos;
            bool m_eos;
        };
    }  // namespace Sio
}  // namespace WireCell
#endif
