#ifndef WIRECELLIMG_JSONBLOBSETSINK
#define WIRECELLIMG_JSONBLOBSETSINK

#include "WireCellIface/IBlobSetSink.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Logging.h"
#include <vector>
#include <string>

namespace WireCell {
    namespace Img {  // fixme move to sio?

        class JsonBlobSetSink : public IBlobSetSink, public IConfigurable {
           public:
            JsonBlobSetSink();
            virtual ~JsonBlobSetSink();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const IBlobSet::pointer& bs);

           private:
            double m_drift_speed;
            std::string m_filename;
            int m_face;
            Log::logptr_t l;
        };
    }  // namespace Img
}  // namespace WireCell
#endif
