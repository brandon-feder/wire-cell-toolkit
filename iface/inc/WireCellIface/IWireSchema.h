/** A wire schema provides information on the
 * detector/anode/face/plane/wire geometry hiearchy.
 *
 * This information should be accessed through this interface instead
 * of directly loading via the WireSchema code so that different
 * components may share the information without leading to multiple
 * loads of the data.
 *
 */

#ifndef WIRECELLIFACE_IWIRESCHEMA
#define WIRECELLIFACE_IWIRESCHEMA

#include "WireCellUtil/IComponent.h"
#include "WireCellUtil/WireSchema.h"

namespace WireCell {

    class IWireSchema : public IComponent<IWireSchema> {
       public:
        virtual ~IWireSchema();

        /// Return the field response data
        virtual const WireSchema::Store& wire_schema_store() const = 0;
    };

}  // namespace WireCell

#endif
