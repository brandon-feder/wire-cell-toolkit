#ifndef WIRECELL_SIMPLETENSORSET
#define WIRECELL_SIMPLETENSORSET

#include "WireCellIface/ITensorSet.h"

namespace WireCell {

    namespace Aux {

        class SimpleTensorSet : public WireCell::ITensorSet {
           public:
            SimpleTensorSet(int ident, Configuration md, ITensor::shared_vector tv)
              : m_ident(ident)
              , m_md(md)
              , m_tv(tv)
            {
            }
            virtual ~SimpleTensorSet() {}

            /// Return some identifier number that is unique to this set.
            virtual int ident() const { return m_ident; }

            /// Optional metadata associated with the tensors
            virtual Configuration metadata() const { return m_md; }

            virtual ITensor::shared_vector tensors() const { return m_tv; }

           private:
            int m_ident;
            Configuration m_md;
            ITensor::shared_vector m_tv;
        };
    }  // namespace Aux
}  // namespace WireCell

#endif
