// hacked version of:
// https://community.intel.com/t5/Intel-oneAPI-Threading-Building/Nondeterministic-processing-order-for-function-node-with/m-p/1164061

#include <tbb/global_control.h>
#include <tbb/flow_graph.h>

#include <iostream>


using namespace tbb::flow;

struct TerminalNode_t {
    continue_msg operator()(int v)
    {
        //std::cerr << "sink: " << v << std::endl;
        if (v != counter)
            std::cerr << "terminal node actual: " << v << ", expected:" << counter << std::endl;
        counter++;
        return continue_msg();
    }
  private:
    int counter = 0;
};

static int const THRESHOLD = 3;
static int const CYCLES = 10000;

int main()
{
    const int threads = 16;

    std::unique_ptr<tbb::global_control> gc;
    if (threads) {
        gc = std::make_unique<tbb::global_control>(
            tbb::global_control::max_allowed_parallelism,
            threads);
    }

    int count = 0;
    
    graph g;
    input_node<int> input(g, [&count](tbb::flow_control& fc) -> int {
        //std::cerr << "source: " << count << std::endl;
        if (count < CYCLES)
        {
            return count++;
        }
        fc.stop();
        return {};
    });
    input.activate();
    
    limiter_node<int> lim( g, THRESHOLD);
    function_node<int, int> func1( g, serial, [](const int& val) -> int {
        //std::cerr << "pass: " << val << std::endl;
        return val;
    } );
    function_node<int, continue_msg> terminal( g, serial, TerminalNode_t() );

#if 0
    queue_node<int> fifo1(g);
    queue_node<int> fifo2(g);
    queue_node<int> fifo3(g);
#else
    sequencer_node<int> fifo1(g, [](const int& val) -> int { return val; });
    sequencer_node<int> fifo2(g, [](const int& val) -> int { return val; });
    sequencer_node<int> fifo3(g, [](const int& val) -> int { return val; });
#endif

    make_edge( lim, func1 );
    make_edge( func1, fifo2 );
    make_edge( fifo2, terminal );
    // make_edge( func1, terminal );
    make_edge( terminal, lim.decrementer() );
    make_edge( input, lim );

    g.wait_for_all();

    std::cerr << "final count: " << count << std::endl;
    return 0;
}
