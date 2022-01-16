/** This component provides field response data as read in from a "WCT
 * field response" JSON file */

#ifndef WIRECELLSIGPROC_FIELDRESPONSE
#define WIRECELLSIGPROC_FIELDRESPONSE

#include "WireCellIface/IFieldResponse.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/Units.h"

namespace WireCell {
    namespace SigProc {
        class FieldResponse : public IFieldResponse, public IConfigurable {
           public:
            // Create directly with the JSON data file or delay that
            // for configuration.
            FieldResponse(const char* frfilename = "");

            virtual ~FieldResponse();

            // IFieldResponse
            virtual const Response::Schema::FieldResponse& field_response() const;

            // IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_fname;
            Response::Schema::FieldResponse m_fr;
        };

    }  // namespace SigProc

}  // namespace WireCell
#endif
