/** A field response provides access to a FieldResponse data
 * structure.  Use this interface instead of directly loading a
 * response schema so that multiple components may share the same data
 * without causing multiple loads.
 */

#ifndef WIRECELLIFACE_IFIELDRESPONSE
#define WIRECELLIFACE_IFIELDRESPONSE

#include "WireCellUtil/IComponent.h"
#include "WireCellUtil/Response.h"

namespace WireCell {

    class IFieldResponse : public IComponent<IFieldResponse> {
       public:
        virtual ~IFieldResponse();

        /// Return the field response data
        virtual const Response::Schema::FieldResponse& field_response() const = 0;
    };

}  // namespace WireCell

#endif
