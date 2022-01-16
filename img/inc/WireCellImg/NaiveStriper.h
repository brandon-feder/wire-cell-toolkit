/** Make stripes based on channel index.

    This component is "naive" because it does not consider dead channels or
    other real world detector pathology.

 */
#ifndef WIRECELLIMG_NAIEVESTRIPER
#define WIRECELLIMG_NAIEVESTRIPER

#include "WireCellIface/ISliceStriper.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Img {

        class NaiveStriper : public ISliceStriper, public IConfigurable {
           public:
            virtual ~NaiveStriper();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& in, output_pointer& out);

           private:
            size_t m_gap;
        };
    }  // namespace Img
}  // namespace WireCell

#endif
