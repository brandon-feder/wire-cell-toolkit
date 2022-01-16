#ifndef WIRECELL_FANINOUT
#define WIRECELL_FANINOUT

#include <boost/signals2.hpp>
#include <deque>
#include <map>

namespace WireCell {

    /** A fanout which takes in Data from a connected slot and buffers it
     * into set of addressable queues.
     */
    template <typename Data, typename Address = int>
    class Fanout {
       public:
        // The fanout is held as a map from address to queue
        typedef Data result_type;
        typedef Address address_type;
        typedef std::deque<result_type> data_queue;
        typedef std::map<int, data_queue> queue_map;

        // Our signal for providers of input data
        typedef boost::signals2::signal<result_type()> signal;
        // The slot type our signal accepts
        typedef typename signal::slot_type slot;

        // Connect a slot to our signal.
        void connect(const slot& s) { m_signal.connect(s); }

        /// Register an address.
        void address(const address_type& addr) { m_fan[addr] = data_queue(); }

        /// Return a data from the given address.  If the address has not
        /// yet been registered it will be but any previously returned
        /// data will not be seen.
        result_type operator()(const address_type& addr)
        {
            data_queue& dq = m_fan[addr];
            if (!dq.size()) {
                result_type dat = *m_signal();
                for (auto it = m_fan.begin(); it != m_fan.end(); ++it) {
                    it->second.push_back(dat);
                }
            }
            result_type dat = dq.front();
            dq.pop_front();
            return dat;
        }

       private:
        signal m_signal;
        queue_map m_fan;
    };

    /** If you have a slot to connect to a Fanout which doesn't
     * inherently care about the address to which it is attached, use
     * the AddressedShunt as a go-between.
     */
    template <typename Data, typename Address = int>
    class Addresser {
       public:
        typedef Data value_type;
        typedef Address address_type;

        // Our signal for providers of input data
        typedef boost::signals2::signal<value_type(const address_type& addr)> signal;
        // The slot type our signal accepts
        typedef typename signal::slot_type slot;

        Addresser(const address_type& addr)
          : m_addr(addr)
        {
        }

        // Connect a slot to our signal.
        void connect(const slot& s) { m_signal.connect(s); }

        // the shunt
        value_type operator()() { return *m_signal(m_addr); }

       private:
        Address m_addr;
        signal m_signal;
    };

    /** Fan-in concept is inherent in boost::signals2, but does
     * require some "combiner" to enact whatever fan-in policy is
     * desired.  This most obvious one is one which synchronizes all
     * input into a simple collection.  A signal like this will return
     * a vector of its inputs.
     *
     *    // a generator returning one higher count each time called
     *    Counter c1(10),c2(20),c3(30);
     *
     *    boost::signals2::signal< int (), Fanin< std::vector<int> > > sig;
     *    sig.connect(c1);
     *    sig.connect(c2);
     *    sig.connect(c3);
     *
     *    sig(); // --> vector(10,20,30)
     */
    template <typename Collection>
    struct Fanin {
        // result_type required for boost::signals2 combiners
        typedef Collection result_type;
        // typedef typename Collection::value_type value_type;

        template <typename InputIterator>
        result_type operator()(InputIterator first, InputIterator last) const
        {
            result_type ret;
            while (first != last) {
                ret.push_back(*first);
                ++first;
            }
            return ret;
        }
    };

}  // namespace WireCell

#endif
