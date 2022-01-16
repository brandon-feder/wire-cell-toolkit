/** BlobGrouping takes in a channel-level cluster and produces another with channels merged ('m' nodes)
 *
 * The input cluster must have (b,w), (c,w) and (s,b) edges and may have (b,b) edges.
 *
 * The output cluster will have (m, b) and (s,b) edges and if the input has (b,b) edges, they are preserved.
 *
 * Grouping is done in the "coarse grained" strategy.
 *
 * See manual for more info.
 */
#ifndef WIRECELL_BLOBGROUPING_H
#define WIRECELL_BLOBGROUPING_H

#include "WireCellIface/IClusterFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

namespace WireCell {

    namespace Img {

        class BlobGrouping : public Aux::Logger, public IClusterFilter, public IConfigurable {
           public:
            BlobGrouping();
            virtual ~BlobGrouping();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& in, output_pointer& out);

           private:
        };

    }  // namespace Img

}  // namespace WireCell

#endif /* WIRECELL_BLOBGROUPING_H */
