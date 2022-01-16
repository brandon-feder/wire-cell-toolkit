/** TupleHelpers help tuples help her make a great meal.
 *
 * These helpers provide template gymnastics to bridge compile-time
 * and run-time access of the collection tuple's.  Compile-time
 * elements are held as types in the tuple, run-time are held as
 * boost::any in a std::vector<boost::any> or other collection.
 *
 */

#ifndef WIRECELLUTIL_TUPLEHELPERS
#define WIRECELLUTIL_TUPLEHELPERS

#include <boost/any.hpp>

#include <tuple>
#include <vector>
#include <deque>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <memory>
#include <cassert>

// much inspiration from:
// https://www.preney.ca/paul/archives/486
// http://loungecpp.wikidot.com/tips-and-tricks:indices
// http://stackoverflow.com/questions/31463388/can-someone-please-explain-the-indices-trick

/// If we are still before C++14, supply the fodder for doing the "indices trick".
#if __cplusplus <= 201103L
namespace std {
    template <std::size_t... Is>
    struct index_sequence {
    };

    template <std::size_t N, std::size_t... Is>
    struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...> {
    };

    template <std::size_t... Is>
    struct make_index_sequence<0, Is...> : index_sequence<Is...> {
    };
}  // namespace std
#else
#include <utility>
#endif  // assume C++14

namespace WireCell {

    /** The basic tuple_helper provides functions to convert a tuple
     * to a vector of boost::any.
     *
     * The doc strings on each helper assume an example tuple type like:
     *
     *     typedef std::tuple<int,float,double,char,std::string> IFDCS;
     *     tuple_helper<IFDCS> helper;
     *
     * For working examples, see test_tuple.cxx.
     *
     */
    template <typename Tuple>
    struct tuple_helper;

    template <template <typename...> class T, typename... Types>
    struct tuple_helper<T<Types...> > {
        typedef T<Types...> tuple_type;

        /// Make a new tuple type which each element is a container
        /// holding type of original tuple element.
        template <template <typename...> class Container>
        struct Wrapped {
            typedef std::tuple<Container<Types>...> type;
        };
        template <template <typename...> class Container>
        struct WrappedConst {
            typedef std::tuple<Container<const Types>...> type;
        };

        typedef std::vector<boost::any> any_vector_type;

        /** Compile-time mapping of methods on tuple element types
         *
         *     std::vector<std::string> typenames = helper.type_names();
         */
        std::vector<std::string> type_names() { return {std::string(typeid(Types).name())...}; }

        // internal
        template <std::size_t... Indices>
        any_vector_type as_any_impl(const tuple_type& t, std::index_sequence<Indices...>)
        {
            return {boost::any(std::get<Indices>(t))...};
        }

        /** Convert a tuple of types to a vector of boost::any.
         *
         *     IFDCS ifdcs{1,2.2f,3.0e-9,'a',"foo"};
         *     std::vector<boost::any> anyvec = helper.as_any(ifdcs);
         */
        any_vector_type as_any(const tuple_type& t)
        {
            return as_any_impl(t, std::make_index_sequence<std::tuple_size<tuple_type>::value>{});
        }

        // internal
        template <std::size_t... Indices>
        tuple_type from_any_impl(const any_vector_type& anyv, std::index_sequence<Indices...>)
        {
            return std::make_tuple(boost::any_cast<Types>(anyv[Indices])...);
        }

        /** Return a tuple made from the contents of an equivalent vector of boost::any
         *
         *  // anyvec is from above example
         *  IFDCS t = helper.from_any(anyvec);
         */
        tuple_type from_any(const any_vector_type& anyv)
        {
            return from_any_impl(anyv, std::make_index_sequence<std::tuple_size<tuple_type>::value>{});
        }

    };  // tuple_helpers

    template <typename T, std::size_t... Indices>
    auto vectorToTupleHelper(const std::vector<T>& v, std::index_sequence<Indices...>)
    {
        return std::make_tuple(v[Indices]...);
    }

    template <std::size_t N, typename T>
    auto vectorToTuple(const std::vector<T>& v)
    {
        assert(v.size() >= N);
        return vectorToTupleHelper(v, std::make_index_sequence<N>());
    }

    /** This convert a tuple of types to vector of deque shared_ptr's
     * of those types.
     */

    template <typename Tuple>
    struct shared_queued;

    template <template <typename...> class T, typename... Types>
    struct shared_queued<T<Types...> > {
        typedef T<Types...> tuple_type;
        // typedef tuple_helper<tuple_type> helper;

        template <template <typename...> class Container>
        struct WrappedShared {
            typedef std::tuple<Container<std::shared_ptr<const Types> >...> type;
        };

        typedef std::deque<boost::any> any_queue_type;

        // This is a tuple<deque<shared_ptr<T1>>, deque<shared_ptr<T2>>, ...>
        typedef typename WrappedShared<std::deque>::type shared_queued_tuple_type;

        // internal, convert a deque<shared_ptr<T>> to a any_queue_type.
        template <typename Element>
        any_queue_type as_any_queue_convert(const std::deque<std::shared_ptr<const Element> >& in)
        {
            return any_queue_type(in.begin(), in.end());
        }

        // internal, index a tuple of typed deque into a vector of any_queues.
        template <std::size_t... Indices>
        std::vector<any_queue_type> as_any_queue_impl(const shared_queued_tuple_type& toq,
                                                      std::index_sequence<Indices...>)
        {
            return {as_any_queue_convert(std::get<Indices>(toq))...};
        }

        /** Convert a tuple of queues of types to a vector of queues of boost::any.
         *
         *     typedef typename tuple_helper<IFDCS>::Wrapped<std::deque>::type IFDCS_queues;
         *     IFDCS_queues qs;
         *     auto any_q = as_any_queue(qs);
         *     cerr << "First element from each queue:\n";
         *     cerr << any_cast<int>(any_q[0][0]) << endl;
         *     cerr << any_cast<float>(any_q[1][0]) << endl;
         *     cerr << any_cast<double>(any_q[2][0]) << endl;
         *     cerr << any_cast<char>(any_q[3][0]) << endl;
         *     cerr << any_cast<std::string>(any_q[4][0]) << endl;
         */
        std::vector<any_queue_type> as_any_queue(const shared_queued_tuple_type& toq)
        {
            return as_any_queue_impl(toq, std::make_index_sequence<std::tuple_size<tuple_type>::value>{});
        }

        // internal, cast an any queue to a deque of shared_ptrs of Type.
        template <typename Type>
        std::deque<std::shared_ptr<const Type> > from_any_queue_convert(const any_queue_type& aq)
        {
            std::deque<std::shared_ptr<const Type> > ret;
            for (auto a : aq) {
                ret.push_back(boost::any_cast<std::shared_ptr<const Type> >(a));
            }
            return ret;
        }

        // internal, index a vector of any queues returning a typed deque<shared_ptr<Type>>.
        template <std::size_t... Indices>
        shared_queued_tuple_type from_any_queue_impl(const std::vector<any_queue_type>& vaq,
                                                     std::index_sequence<Indices...>)
        {
            return std::make_tuple(from_any_queue_convert<Types>(vaq[Indices])...);
        }

        /** Convert a vector of queues of any to a tuple of queues of type.
         */
        shared_queued_tuple_type from_any_queue(const std::vector<any_queue_type>& vaq)
        {
            return from_any_queue_impl(vaq, std::make_index_sequence<std::tuple_size<tuple_type>::value>{});
        }

    };  // shared_queued

    // http://stackoverflow.com/a/20711990
    /** Make an N-tuple of elements all the same type
     *
     * type_repeater<3, float>::type blah;
     */
    template <std::size_t N, typename T>
    struct type_repeater {
        typedef decltype(std::tuple_cat(std::tuple<T>(), typename type_repeater<N - 1, T>::type())) type;
    };
    template <typename T>
    struct type_repeater<0, T> {
        typedef decltype(std::tuple<>()) type;
    };

}  // namespace WireCell

#endif
