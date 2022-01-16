#ifndef WIRECELL_GENERATORITER
#define WIRECELL_GENERATORITER

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace WireCell {

    // template <typename Callable, typename Value>
    // struct NullGenerator : public Callable {
    // 	NullGenerator() {}
    // 	~NullGenerator() {}
    // 	Value operator()() { return 0; }
    // 	operator bool() const { return false;}
    // 	bool operator==(const NullGenerator& rhs) const { return true; }
    // 	bool operator==(const Callable& rhs) const { return true; }
    // };

    /** A generator iterator using a Callable.
     *
     * Requirements on Callable
     * - returns Value.
     * - evaluates as false when it is not exhausted, true otherwise.
     * - copy-by-value, copies are independent.
     * - instances must be testable for equality.
     *
     * Inspired by:
     * http://stackoverflow.com/questions/27604201/implement-lazy-generator-as-forward-iterator-in-c
     */
    template <typename Callable, typename Value>
    struct GeneratorIter
      : public boost::iterator_facade<GeneratorIter<Callable, Value>, Value, boost::forward_traversal_tag> {
        GeneratorIter(const Callable& func)
          : count(0)
          , func(func)
          , last_val(0)
        {
            advance();
        }
        void advance()
        {
            if (func) {
                last_val = func();
            }
        }
        Value operator*() const { return last_val; }
        GeneratorIter& operator++()
        {
            advance();
            return *this;
        }
        GeneratorIter operator++(int)
        {
            GeneratorIter res = *this;
            advance();
            return res;
        }
        bool operator==(const GeneratorIter& rhs) const
        {
            return (!func && !rhs.func) || (func == rhs.func && count == rhs.count);
        }
        bool operator!=(const GeneratorIter& rhs) const { return !(*this == rhs); }

        operator bool() const { return func; }

        size_t count;
        Callable func;
        Value last_val;
    };

}  // namespace WireCell

#endif
