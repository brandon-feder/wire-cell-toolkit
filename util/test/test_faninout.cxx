#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Faninout.h"

#include <boost/signals2.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <vector>

using namespace std;

struct Counter {
    int count;
    Counter(int c = 0)
      : count(c)
    {
    }
    int operator()() { return count++; }
};

struct Echoer {
    int operator()(int x) { return x; }
};

/// moved to templated version in Faninout.h
// struct Addresser {
//     int addr;
//     typedef boost::signals2::signal<int (int)> signal;
//     typedef signal::slot_type slot_type;
//     signal sig;
//     Addresser(int address) : addr(address) {}
//     int operator()() {
// 	int val = *sig(addr);
// 	cerr << "Address "<<addr<<" returns " << val << endl;
// 	return val;
//     }
// };

struct Consumer {
    typedef boost::signals2::signal<int()> signal;
    typedef signal::slot_type slot_type;
    signal sig;
    int operator()()
    {
        int val = *sig();
        cerr << "Consumed: " << val << endl;
        return val;
    }
};

using namespace std;
using namespace WireCell;

void test_plug_and_play()
{
    Counter counter;
    Fanout<int> fanout;
    fanout.connect(counter);

    int addresses[] = {1, 2, 3, 0, 0, 0, 2, 4, 4, 4, 4, 4, 1, 1, 2, 1, 1, -1};
    int want_val[] = {0, 1, 2, 3, 4, 5, 2, 0, 1, 2, 3, 4, 1, 2, 3, 3, 4, -1};

    // pre-register
    fanout.address(4);

    for (int ind = 0; addresses[ind] >= 0; ++ind) {
        int addr = addresses[ind];
        if (addr < 0) {
            break;
        }
        int want = want_val[ind];
        int got = fanout(addr);
        cerr << "addr=" << addr << " got:" << got << " want:" << want << endl;
        Assert(want == got);
    }
}

void test_ref()
{
    Echoer echo;

    Addresser<int> addr0(0), addr1(1);
    addr0.connect(echo);
    addr1.connect(echo);

    Consumer consumer0, consumer1;
    consumer0.sig.connect(boost::ref(addr0));
    consumer1.sig.connect(boost::ref(addr1));

    cerr << consumer0() << endl;
    cerr << consumer1() << endl;
}

void test_fanout_addressing()
{
    Counter counter;
    Fanout<int> fanout;
    fanout.address(0);
    fanout.address(1);
    fanout.connect(counter);

    Addresser<int> addr0(0), addr1(1);
    addr0.connect(boost::ref(fanout));
    addr1.connect(boost::ref(fanout));

    Consumer consumer0, consumer1;
    consumer0.sig.connect(boost::ref(addr0));
    consumer1.sig.connect(boost::ref(addr1));

    cerr << consumer1() << endl;
    cerr << consumer0() << endl;
    cerr << consumer0() << endl;
    cerr << consumer0() << endl;
    cerr << consumer1() << endl;
    cerr << consumer0() << endl;
}

int test_fanin()
{
    Counter c1(10), c2(20), c3(30);

    // Fan-in concept is inherent in boost::signals2, but does require
    // some "combiner" to enact whatever fan-in policy is desired.
    // This one synchronizes input into a collection.
    boost::signals2::signal<int(), Fanin<std::vector<int> > > sig;
    sig.connect(c1);
    sig.connect(c2);
    sig.connect(c3);

    for (int ind = 0; ind < 10; ++ind) {
        auto vec = sig();
        cerr << ind;
        for (auto x : vec) {
            cerr << " " << x;
        }
        cerr << endl;
    }

    return 0;
}

int main()
{
    test_plug_and_play();
    test_ref();
    test_fanout_addressing();
    test_fanin();
    return 0;
}
