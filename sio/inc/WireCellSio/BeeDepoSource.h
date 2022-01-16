/*
  This component will provide depositions from a JSON file following
  the same schema used to supply Bee.

  It provides a number of JSON arrays, one for each attribute of a
  depo.

{
"runNo":"1",
"subRunNo":"0",
"eventNo":"1",
"geom":"protodune",
"x":[-377.0,-377.3,-32.1,-32.1,...
"y":[200.5,200.4,422.5,422.5,42...
"z":[39.8,39.7,-0.5,-0.4,-0.4,-...
"q":[1686,1510,1315,1381,1323,1...
"nq":[1,1,1,1,1,1,1,1,1,1,1,1,1...
"type":"truthDepo"
}

  The  "nq" and the non-array atributes are ignored.

  The component is configured with a collection of input files.  These
  may be compressed JSON or Jsonnet files.


 */

#ifndef WIRECELLSIO_JSONDEPOSOURCE
#define WIRECELLSIO_JSONDEPOSOURCE

#include "WireCellIface/IDepoSource.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Sio {

        class BeeDepoSource : public IDepoSource, public IConfigurable {
           public:
            BeeDepoSource();
            virtual ~BeeDepoSource();

            /// IDepoSource
            virtual bool operator()(IDepo::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

           private:
            std::vector<std::string> m_filenames;
            std::string m_policy;
            IDepo::vector m_depos;  // current set of depos
        };
    }  // namespace Sio
}  // namespace WireCell
#endif
