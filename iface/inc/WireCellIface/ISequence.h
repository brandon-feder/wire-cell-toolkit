#ifndef WIRECELL_ISEQUENCE
#define WIRECELL_ISEQUENCE

#include "WireCellUtil/IteratorBase.h"

#include <memory>

namespace WireCell {

    /** Abstract base class interface for sequence of data providing
     * facade iterators over abstract base iterators.
     *
     * If a subclass multiply-inherits then one has to do
     * myobj->ISequence<IThisData>::begin() etc or cast to the desired
     * interface.
     */
    template <class IDataClass>
    class ISequence {  // note: not a WireCell::Interface
       public:
        typedef ISequence<IDataClass> this_type;

        /// Access this sequence via shared (non-const) pointer.
        typedef std::shared_ptr<this_type> pointer;

        typedef typename IDataClass::iterator iterator;
        typedef typename IDataClass::const_iterator const_iterator;
        typedef typename IDataClass::base_iterator base_iterator;
        typedef typename IDataClass::iterator_range iterator_range;

        /// Adapt one iterator to the standard facade iterator.
        template <typename OtherIter>
        static iterator adapt(const OtherIter& itr)
        {
            return iterator(IteratorAdapter<OtherIter, base_iterator>(itr));
        }
        template <typename OtherIter>
        static const_iterator cadapt(const OtherIter& itr)
        {
            return const_iterator(IteratorAdapter<OtherIter, base_iterator>(itr));
        }

        /// Concrete class must implement:
        virtual const_iterator cbegin() const = 0;
        virtual const_iterator cend() const = 0;
        virtual const_iterator begin() const { return cbegin(); }
        virtual const_iterator end() const { return cbegin(); }
        virtual iterator begin() { return iterator(cbegin()); }
        virtual iterator end() { return iterator(cbegin()); }

        /// Return begin/end pair as iterator range.
        virtual iterator_range range() { return iterator_range(begin(), end()); }

        virtual ~ISequence();
    };

    /** An ISequence made by adapting begin/end iterators of some
     * other type.
     */
    template <class IDataClass>
    class SequenceAdapter : public ISequence<IDataClass> {
       public:
        typedef typename ISequence<IDataClass>::iterator iterator;

        template <typename OtherIter>
        SequenceAdapter(const OtherIter& begin, const OtherIter& end)
          : m_begin(ISequence<IDataClass>::adapt(begin))
          , m_end((ISequence<IDataClass>::adapt(end)))
        {
        }

        virtual iterator begin() { return m_begin; }
        virtual iterator end() { return m_end; }

        virtual ~SequenceAdapter();

       private:
        iterator m_begin, m_end;
    };

}  // namespace WireCell

#endif
