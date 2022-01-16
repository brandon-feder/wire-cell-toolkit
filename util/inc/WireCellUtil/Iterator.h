#ifndef WIRECELL_IWIREDATABASE
#define WIRECELL_IWIREDATABASE

#include "WireCellUtil/IteratorBase.h"
#include <iterator>

namespace WireCell {

    /** This iterator provides a facade over a WireCell::IteratorBase<ValueType>.
     *
     * This facade can be passed by value without slicing the abstract
     * base iterator.
     */
    template <typename ValueType>
    class Iterator : public std::iterator<std::forward_iterator_tag, ValueType> {
       public:
        typedef WireCell::IteratorBase<ValueType> BaseIteratorType;

        // Produce an empty/invalid iterator.
        Iterator()
          : base_itr(0)
        {
        }

        Iterator(const Iterator& other)
          : base_itr(0)
        {
            if (other.base_itr) {
                base_itr = other.base_itr->clone();
            }
        }
        Iterator(const BaseIteratorType& base_other)
          : base_itr(0)
        {
            base_itr = base_other.clone();
        }
        ~Iterator()
        {
            if (base_itr) {
                delete base_itr;
            }
            base_itr = 0;
        }

        bool operator==(const Iterator& rhs) const
        {
            if (base_itr == rhs.base_itr) return true;
            if (!base_itr || !rhs.base_itr) return false;
            return *base_itr == *rhs.base_itr;
        }
        bool operator!=(const Iterator& rhs) const
        {
            if (base_itr && rhs.base_itr) {
                return *base_itr != *rhs.base_itr;
            }
            return base_itr != rhs.base_itr;
        }

        Iterator& operator++()
        {
            ++(*base_itr);
            return *this;
        }

        ValueType operator*() const { return *(*base_itr); }

        Iterator& operator=(const Iterator& rhs)
        {
            if (base_itr) {
                delete base_itr;
            }
            base_itr = rhs.base_itr->clone();
            return *this;
        }

       private:
        BaseIteratorType* base_itr;
    };

}  // namespace WireCell

#endif
