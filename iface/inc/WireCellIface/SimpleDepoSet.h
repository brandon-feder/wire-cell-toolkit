#include "WireCellIface/IDepoSet.h"

namespace WireCell {

    class SimpleDepoSet : public IDepoSet {
        int m_ident;
        IDepo::shared_vector m_depos;

       public:
        SimpleDepoSet(int ident, const IDepo::vector& depos)
          : m_ident(ident)
          , m_depos(std::make_shared<IDepo::vector>(depos.begin(), depos.end()))
        {
        }
        virtual ~SimpleDepoSet();
        virtual int ident() const { return m_ident; }
        virtual IDepo::shared_vector depos() const { return m_depos; }
    };

}  // namespace WireCell
