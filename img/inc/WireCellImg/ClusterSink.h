#ifndef WIRECELLIMG_CLUSTERSINK
#define WIRECELLIMG_CLUSTERSINK

#include "WireCellIface/IClusterSink.h"
#include "WireCellIface/IConfigurable.h"

#include <string>

namespace WireCell {
    namespace Img {

        class ClusterSink : public IClusterSink, public IConfigurable {
           public:
            ClusterSink();
            virtual ~ClusterSink();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const ICluster::pointer& cluster);

           private:
            std::string m_filename;
            std::string m_node_types;
        };
    }  // namespace Img
}  // namespace WireCell
#endif
