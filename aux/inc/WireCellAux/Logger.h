/** A base class to help normalize logging.

    All components which log are encouraged to inherit from
    Aux::Logger.

    To convert a component from using its own logger directly to
    participating in named logging follow these steps.

    1) In your .h file change:

    #include "WireCellUtil/Logging.h"

    to

    #include "WireCellAux/Logger.h"

    2) Add "public Aux::Logger" in the class ineritance chain.

    3) Delete the old Log::logptr_t data member from the class def.

    4) In your .cxx constructor delete initialization of the old
    logger member and initialize the base class by adding:

        Aux::Logger("TypeName", "grp") 

    to the start of the constructor's initialization list.  Replace
    "grp" with sub-package short name or other short identifier (eg
    "img" or also "glue" is conventionally used for fans regardless of
    what package provides them).

    5) At top of .cxx, add WireCell::INamed in the WIRECELL_FACTORY()
    macro list of interfaces.

    6) In body of .cxx code replace use of old data member with "log"
    (if not already using that variable name).

    7) In all message body strings remove any class identifier strings
    as they are not provided by the new "log".
 */

#ifndef WIRECELLAUX_LOGGER
#define WIRECELLAUX_LOGGER

#include "WireCellUtil/Logging.h"
#include "WireCellIface/INamed.h"

namespace WireCell::Aux {

    /** A logger base.
     *
     *  This makes the component an INamed
     */
    class Logger : virtual public INamed {
        std::string m_iname, m_tname, m_gname;

      public:

        // Pass in the concrete configurable name such as given to
        // WIRECELL_FACTORY.  The group name is recomended to be the
        // short sub package name (eg "gen", "sigproc" or "img").
        Logger(const std::string& type_name,
               const std::string& group_name = "aux");
        virtual ~Logger();

        // INamed interface
        virtual void set_name(const std::string& name);
        virtual std::string get_name() const;


      protected:
        
        // Subclasses are encouraged to use this "log" instead of
        // making their own.
        WireCell::Log::logptr_t log;

    };
}

#endif
