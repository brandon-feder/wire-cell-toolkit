#ifndef WIRECELL_IDATA
#define WIRECELL_IDATA

#include <memory>  // std::shared_ptr
#include <vector>
namespace WireCell {

    template <class Type>
    class IData {
       public:
        virtual ~IData() {}

        typedef Type value_type;

        /// Never expose the basic pointer-to-object, but rather only
        /// through shared, const pointers.
        typedef std::shared_ptr<const Type> pointer;

        typedef std::vector<pointer> vector;
        typedef std::shared_ptr<const vector> shared_vector;

        // /// Abstract base iterator
        // typedef IteratorBase<pointer> base_iterator;

        // /// The facade-iterator which wraps an instance of derived abstract iterator.
        // typedef Iterator<pointer> iterator;

        // /// A range of iterators.
        // typedef boost::iterator_range<iterator> iterator_range;
    };
}  // namespace WireCell

// http://antonym.org/2014/02/c-plus-plus-11-range-based-for-loops.html
// http://stackoverflow.com/questions/8542591/c11-reverse-range-based-for-loop

#endif
