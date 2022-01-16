#ifndef WIRECELLUTIL_RANDOMITER
#define WIRECELLUTIL_RANDOMITER

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace WireCell {

    template <typename Container, typename Value>
    struct RandomIter
      : public boost::iterator_facade<RandomIter<Container, Value>, Value, boost::random_access_traversal_tag> {
        // Constructor for begin()
        explicit RandomIter(Container& array)
          : m_array(array)
          , m_index(0)
        {
        }

        // Constructor for end(), bool is just a maker
        explicit RandomIter(bool, Container& array)
          : m_array(array)
          , m_index(array.size())
        {
        }

       private:
        friend class boost::iterator_core_access;

        Container& m_array;
        int m_index;

        bool equal(RandomIter<Container, Value> const& other) const { return this->m_index == other.m_index; }

        typedef boost::iterator_facade<RandomIter<Container, Value>, Value, boost::random_access_traversal_tag> facade;
        typename facade::difference_type distance_to(RandomIter const& other) const
        {
            return other.m_index - this->m_index;
        }

        void advance(typename facade::difference_type n)
        {
            m_index += n;
            if (m_index >= (int) m_array.size() || m_index < 0) {
                m_index = m_array.size();
            }
        }

        void increment() { advance(1); }

        void decrement() { advance(-1); }

        // const and non-const dereference of this iterator
        Value& dereference() const { return m_array.at(m_index); }
    };

}  // namespace WireCell

#endif
