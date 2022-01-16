#ifndef WIRECELLTBB_TBBFLOW
#define WIRECELLTBB_TBBFLOW

#include "WireCellIface/IApplication.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDataFlowGraph.h"
#include "WireCellUtil/DfpGraph.h"
#include "WireCellAux/Logger.h"

namespace WireCellTbb {

    class TbbFlow : public WireCell::Aux::Logger,
                    public WireCell::IApplication,
                    public WireCell::IConfigurable
    {
        WireCell::IDataFlowGraph::pointer m_dfp;
        WireCell::DfpGraph m_dfpgraph;

      public:
        TbbFlow();
        virtual ~TbbFlow();

        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;

        virtual void execute();

    };
}  // namespace WireCellTbb

#endif
