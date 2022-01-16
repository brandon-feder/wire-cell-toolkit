#include <tbb/flow_graph.h>
#include <iostream>
#include <tuple>

using namespace std;
namespace dfp = tbb::flow;

template <typename DataType>
struct number_source {
    typedef DataType data_type;
    typedef std::vector<data_type> vector_type;
    vector_type dat;
    number_source(const vector_type& d)
      : dat(d)
    {
    }
    data_type operator()(tbb::flow_control& fc)
    {
        cerr << "number_source with " << dat.size() << " element " << endl;
        if (dat.empty()) {
            cerr << "\tempty" << endl;
            fc.stop();
            return {};
        }
        auto out = dat.front();
        dat.erase(dat.begin());
        cerr << "\treturning value: " << out << endl;
        return out;
    }
};

typedef dfp::multifunction_node<int, std::tuple<float> > int2float_node;

struct I2Fcaster {
    void operator()(const int& in, int2float_node::output_ports_type& op)
    {
        float f(in);
        bool ok = std::get<0>(op).try_put(f);
        if (ok) {
            cerr << "cast " << in << " to " << f << endl;
        }
        else {
            cerr << "FAILED " << in << " to " << f << endl;
        }
    }
};

#include <chrono>
#include <thread>
void msleep(int msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }

int main()
{
    dfp::graph g;
    std::vector<int> numbers{5, 4, 3, 2, 1, 0};
    dfp::input_node<int> number_input_node(g, number_source<int>(numbers));
    dfp::function_node<int, int> int_chirp_node(g, dfp::unlimited, [](const int& v) {
        cerr << "i value: " << v << endl;
        msleep(v * 100);
        cerr << "...woke, returning i=" << v << endl;
        return v;
    });
    dfp::function_node<float, float> float_chirp_node(g, dfp::unlimited, [](const float& v) {
        cerr << "f value: " << v << endl;
        msleep(v * 100);
        cerr << "...woke, returning f=" << v << endl;
        return v;
    });
    int2float_node i2fcaster_node(g, dfp::unlimited, I2Fcaster());

    cerr << "make edges" << endl;
    make_edge(number_input_node, int_chirp_node);
    make_edge(int_chirp_node, i2fcaster_node);
    make_edge(dfp::output_port<0>(i2fcaster_node), float_chirp_node);

    cerr << "Activate source" << endl;
    number_input_node.activate();
    cerr << "Waiting for graph" << endl;
    g.wait_for_all();
    return 0;
}
