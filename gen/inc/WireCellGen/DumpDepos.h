#ifndef WIRECELL_DUMPDEPOS
#define WIRECELL_DUMPDEPOS

#include "WireCellIface/IDepoSink.h"

namespace WireCell {

    class DumpDepos : public IDepoSink {
       public:
        DumpDepos();
        virtual ~DumpDepos();
        virtual bool operator()(const IDepo::pointer& depo);

       private:
        int m_nin;
    };

}  // namespace WireCell

#endif
