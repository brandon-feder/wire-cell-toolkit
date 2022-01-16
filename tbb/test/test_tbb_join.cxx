/**
   This is a generic (no WC) test of how one can use the
   tbb::flow::join_node in a way that breaks out the input ports of
   the node into a std::vector.  This test patterns needed to make the
   graph edges in a dynamic rather than static/compiled manner.
 */

#include <tbb/flow_graph.h>

#include <iostream>
using namespace std;

struct CountDown {
    const int index;
    int count;
    CountDown(int index, int n = 10)
      : index(index)
      , count(n)
    {
        cerr << "CountDown(" << index << " , " << n << ")\n";
    }
    int operator()(tbb::flow_control& fc)
    {
        if (!count) {
            cerr << "CountDown(" << index << "): EOS\n";
            fc.stop();
            return {};
        }
        auto result = count--;
        cerr << "CountDown(" << index << "): " << result<< endl;
        return result;
    }
};

struct MyJoin {
    bool operator()(const vector<int>& in, int& out)
    {
        out = 0;
        for (auto x : in) {
            out += x;
        }
        return true;
    }
};

struct Chirp {
    void operator()(const int& x) { cerr << x << endl; }
};

// Get receivers of a join node in vector.
typedef tbb::flow::receiver<int> int_receiver;

template <class TupleType, int N>
struct TupleHelper {
    TupleHelper<TupleType, N - 1> nm1helper;

    vector<int_receiver*> input_ports(tbb::flow::join_node<TupleType>& jn)
    {
        vector<int_receiver*> ret = nm1helper.input_ports(jn);
        int_receiver* rec = dynamic_cast<int_receiver*>(&tbb::flow::input_port<N - 1>(jn));
        ret.insert(ret.begin(), rec);
        return ret;
    }

    vector<int> values(const TupleType& t)
    {
        vector<int> ret = nm1helper.values(t);
        int val = std::get<N - 1>(t);
        ret.insert(ret.begin(), val);
        return ret;
    }

    // breakout tuple->vector to make this a function_node body
    vector<int> operator()(const TupleType& t) { return values(t); }
};

template <class TupleType>
struct TupleHelper<TupleType, 0> {
    vector<int_receiver*> input_ports(tbb::flow::join_node<TupleType>& jn) { return vector<int_receiver*>(); }
    vector<int> values(const TupleType& t) { return vector<int>(); }
};

// need a way to go from a number N to a collection of receivers
// corresponding to a tbb::flow::join_node input and the sender
// corresponding to a IJoinNode's output.  N is given by the
// IJoinNode.

struct Adder {
    int operator()(const vector<int>& in)
    {
        int tot = 0;
        std::string comma = "";
        cerr << "Adding: ";
        for (auto x : in) {
            tot += x;
            cerr << comma << x;
            comma = " + ";
        }
        cerr << " = " << tot << "\n";
        return tot;
    }
};

int main()
{
    tbb::flow::graph graph;

    typedef tbb::flow::input_node<int> int_source;
    vector<int_source> countdowns;

    int n = 3;  // explicitly nonconst
    for (int i = 0; i < n; ++i) {
        countdowns.push_back(int_source(graph, CountDown(i)));
    }

    // join
    typedef std::tuple<int, int, int> IntTriple;
    TupleHelper<IntTriple, 3> th;

    typedef tbb::flow::join_node<IntTriple> JoinInt3;
    JoinInt3 jn(graph);
    vector<int_receiver*> jrec = th.input_ports(jn);

    tbb::flow::function_node<std::tuple<int, int, int>, vector<int> > bo(graph, 0, th);
    tbb::flow::function_node<vector<int>, int> fn(graph, 0, Adder());

    for (int i = 0; i < n; ++i) {
        make_edge(countdowns[i], *jrec[i]);
    }
    make_edge(jn, bo);
    make_edge(bo, fn);
    for (int i = 0; i < n; ++i) {
        countdowns[i].activate();
    }

    graph.wait_for_all();
}
