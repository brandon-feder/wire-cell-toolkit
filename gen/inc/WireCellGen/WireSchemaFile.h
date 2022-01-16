/**
   A wire schema store based on loading data from files.
 */

#ifndef WIRECELL_GEN_WIRESCHEMAFILE
#define WIRECELL_GEN_WIRESCHEMAFILE

#include "WireCellIface/IWireSchema.h"
#include "WireCellIface/IConfigurable.h"

#include <string>

namespace WireCell {
    namespace Gen {

        class WireSchemaFile : public IWireSchema, public IConfigurable {
           public:
            WireSchemaFile(const char* frfilename = "");
            virtual ~WireSchemaFile();

            virtual const WireSchema::Store& wire_schema_store() const;

            // IConfigurable
            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

           private:
            std::string m_fname;
            WireSchema::Store m_store;
        };
    }  // namespace Gen
}  // namespace WireCell

#endif
