#ifndef WIRECELL_IDEPOSET
#define WIRECELL_IDEPOSET

#include "WireCellIface/IDepo.h"

namespace WireCell {

    /** An interface to information about a deposition of charge.
     */
    class IDepoSet : public IData<IDepoSet> {
       public:
        virtual ~IDepoSet();

        /// Return some identifier number that is unique to this set.
        virtual int ident() const = 0;

        /// Return the depositions in this set.
        virtual IDepo::shared_vector depos() const = 0;
    };
}  // namespace WireCell

#endif
