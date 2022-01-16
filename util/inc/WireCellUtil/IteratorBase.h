#ifndef WIRECELL_ITERATORBASE
#define WIRECELL_ITERATORBASE

#include <vector>

namespace WireCell {

    /** An abstract base class for an iterator providing access to an
     * object of type VType.
     *
     * See WireCell::Iterator for what to expose to the client in your
     * class/interface API.
     */
    template <typename ValueType>
    class IteratorBase {
       public:
        typedef ValueType value_type;

        virtual ~IteratorBase() {}

        virtual bool operator==(const IteratorBase& rhs) const = 0;
        virtual bool operator!=(const IteratorBase& rhs) const = 0;
        virtual IteratorBase& operator=(const IteratorBase& rhs) = 0;
        virtual IteratorBase& operator++() = 0;
        virtual value_type operator*() const = 0;

        virtual IteratorBase* clone() const = 0;
    };

    /** Handy adapter from an  iterator to an abstract base iterator.
     *
     * Eg:
     *
     * class MyData : virtual public IData {...};
     * typedef IteratorBase< const IData* > my_base_iterator;
     * typedef std::vector<MyData*> MyStore;
     * typedef IteratorAdapter< MyStore::iterator, my_base_iterator > my_iterator;
     *
     * typedef std::Iterator<const IMyData*> data_iterator;
     * typedef std::pair<data_iterator, data_iterator> data_range;
     *
     * In interface class:
     *
     * data_range get_data() const = 0;
     *
     * In implementation with
     *
     *   MyStore m_store;
     *
     * data_range MyImp::get_data() {
     *    return data_range(my_iterator(m_store.begin(),
     *                      my_iterator(m_store.end()));
     * }
     *
     * Reasons not to use this adapter and go to the trouble to write
     * your own iterator class:
     *
     * - want to not use up memory making all objects up front
     *
     * - can generate them on the fly fast enough.
     *
     * - want lazy data access or otherwise, have the iterator "phone
     * home" to some other container or source of data.
     *
     * - simply not using a STL container or something that already
     * has std::iterators to access the underlying collection.
     *
     */
    template <typename adapted_iterator, typename base_iterator>
    class IteratorAdapter : public base_iterator {
       public:
        typedef typename base_iterator::value_type value_type;

        IteratorAdapter(adapted_iterator it)
          : m_it(it)
        {
        }
        virtual ~IteratorAdapter() {}

        const IteratorAdapter& dc(const base_iterator& other) const
        {
            return *dynamic_cast<const IteratorAdapter*>(&other);  // segfault on type mismatch
        }
        bool operator==(const base_iterator& rhs) const { return m_it == dc(rhs).m_it; }
        bool operator!=(const base_iterator& rhs) const { return m_it != dc(rhs).m_it; }
        base_iterator& operator=(const base_iterator& rhs)
        {
            m_it = dc(rhs).m_it;
            return *this;
        }
        base_iterator& operator++()
        {
            ++m_it;
            return *this;
        }
        value_type operator*() const { return *m_it; }

        base_iterator* clone() const { return new IteratorAdapter(m_it); }

       private:
        adapted_iterator m_it;
    };

}  // namespace WireCell

#endif
