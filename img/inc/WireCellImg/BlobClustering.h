/** Cluster blobs.

    This takes a stream of IBlobSets and mints a new ICluster on EOS
    or earlier if a gap in time slice is found.

    It assumes each blob set represents all blobs found in one time
    slice (including none) and that blob sets are delivered in time
    order.  Blobs in the set may span both faces of an anode plane.
    Gaps in time between blob sets will be determined by the set's
    slice.

    Clusters will not span a gap.  Likewise, when an EOS is
    encountered, all clusters are flushed to the output queue.  If the
    "spans" config param is 0 then no gap testing is done.

    The produced ICluster has b, w, c and s nodes.

    Note, that input blob sets and thus their blobs may be held
    between calls (via their shared pointers).

 */

#ifndef WIRECELLIMG_BLOBCLUSTERING
#define WIRECELLIMG_BLOBCLUSTERING

#include "WireCellIface/IClustering.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IWire.h"
#include "WireCellIface/IChannel.h"
#include "WireCellIface/IBlob.h"
#include "WireCellIface/ISlice.h"
#include "WireCellIface/IBlobSet.h"

#include "WireCellUtil/IndexedGraph.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Img {

        class BlobClustering : public Aux::Logger, public IClustering, public IConfigurable {
           public:
            BlobClustering();
            virtual ~BlobClustering();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& blobset, output_queue& clusters);

           private:
            // User may configure how many slices can go missing
            // before breaking the clusters.  Default is 1.0, slices
            // must be adjacent in time or clusters will flush.
            double m_spans;

            // for judging gap and spatial clustering.
            IBlobSet::pointer m_last_bs;

            cluster_indexed_graph_t m_grind;

            // internal methods
            void add_slice(const ISlice::pointer& islice);
            void add_blobs(const input_pointer& newbs);

            // flush graph to output queue
            void flush(output_queue& clusters);

            // Add the newbs to the graph.  Return true if a flush is needed (eg, because of a gap)
            bool graph_bs(const input_pointer& newbs);

            // return true if a gap exists between the slice of newbs and the last bs.
            bool judge_gap(const input_pointer& newbs);

            // Blob set must be kept, this saves them.
            void intern(const input_pointer& newbs);

        };
    }  // namespace Img
}  // namespace WireCell

#endif
