/** This simply collects input blob sets into an output vector based
 * on them having the same slice. */

#ifndef WIRECELL_IMG_BLOBSETSYNC
#define WIRECELL_IMG_BLOBSETSYNC

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IBlobSetFanin.h"
#include "WireCellAux/Logger.h"

namespace WireCell {
    namespace Img {

        class BlobSetSync : public Aux::Logger, public IBlobSetFanin, public IConfigurable {
           public:
            BlobSetSync();
            virtual ~BlobSetSync();

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual std::vector<std::string> input_types();
            virtual bool operator()(const input_vector& invec, output_pointer& out);

           private:
            size_t m_multiplicity;
        };
    }  // namespace Img
}  // namespace WireCell

#endif
