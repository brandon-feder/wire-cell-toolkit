/*
 This test requires TBB to be built with C++11 support which to really
 make happens requires a patch to its build system at least as of
 tbb44_20151115oss something like:

$ diff -u build/linux.gcc.inc{~,}
--- build/linux.gcc.inc~	2015-11-25 05:48:43.000000000 -0500
+++ build/linux.gcc.inc	2016-01-07 17:56:54.594682208 -0500
@@ -42,6 +42,10 @@
 ifneq (,$(shell gcc -dumpversion | egrep  "^(4\.[4-9]|[5-9])"))
     CPP11_FLAGS = -std=c++0x -D_TBB_CPP0X
 endif
+# gcc 4.8 and higher support -std=c++11
+ifneq (,$(shell gcc -dumpversion | egrep  "^(4\.[8-9]|[5-9])"))
+    CPP11_FLAGS = -std=c++11 -D_TBB_CPP0X
+endif

 # gcc 4.2 and higher support OpenMP
 ifneq (,$(shell gcc -dumpversion | egrep  "^(4\.[2-9]|[5-9])"))



*/

#include <tbb/flow_graph.h>

#include <iostream>
#include <tuple>

struct CountDown {
    const int index;
    int count;
    CountDown(int index, int n = 10)
      : index(index)
      , count(n)
    {
        std::cerr << "CountDown(" << index << " , " << n << ")\n";
    }
    bool operator()(int &x)
    {
        if (!count) {
            std::cerr << "CountDown(" << index << "): EOS\n";
            return false;
        }
        x = count--;
        std::cerr << "CountDown(" << index << "): " << x << std::endl;
        return true;
    }
};

class Adder : public tbb::flow::composite_node<std::tuple<int, int>, std::tuple<int> > {
    tbb::flow::join_node<std::tuple<int, int>, queueing> j;
    tbb::flow::function_node<std::tuple<int, int>, int> f;
    typedef tbb::flow::composite_node<std::tuple<int, int>, std::tuple<int> > base_type;

    struct f_body {
        int operator()(const std::tuple<int, int> &t)
        {
            int n = (std::get<1>(t) + 1) / 2;
            int sum = std::get<0>(t) + std::get<1>(t);
            std::cout << "Sum of the first " << n << " positive odd numbers is  " << n << " squared: " << sum
                      << std::endl;
            return sum;
        }
    };

   public:
    Adder(tbb::flow::graph &g)
      : base_type(g)
      , j(g)
      , f(g, tbb::flow::unlimited, f_body())
    {
        make_edge(j, f);
        base_type::input_ports_type input_tuple(input_port<0>(j), input_port<1>(j));
        base_type::output_ports_type output_tuple(f);
        base_type::set_external_ports(input_tuple, output_tuple);
    }
};
