/** This tiling algorithm makes use of RayGrid for the heavy lifting.
 *
 * It must be configured with a face on which to focus.  Only wires
 * (segments) in that face which are connected to channels with data
 * in a slice will be considered when tiling.
 *
 * It does not "know" about dead channels.  If your detector has them
 * you may place a component upstream which artifically inserts
 * non-zero signal on dead channels in slice input here.
 *
 * The resulting IBlobs have ident numbers which increment starting
 * from zero.  The ident is reset when EOS is received.
 */
#ifndef WIRECELLIM_GRIDTILING
#define WIRECELLIM_GRIDTILING

#include "WireCellIface/ITiling.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IAnodeFace.h"

#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Img {

        class GridTiling : public Aux::Logger, public ITiling, public IConfigurable {
           public:
            GridTiling();
            virtual ~GridTiling();
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& slice, output_pointer& blobset);

           private:
            size_t m_blobs_seen;
            IAnodePlane::pointer m_anode;
            IAnodeFace::pointer m_face;
            double m_threshold;

        };
    }  // namespace Img
}  // namespace WireCell

#endif
