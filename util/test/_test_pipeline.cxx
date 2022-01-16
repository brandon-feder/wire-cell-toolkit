#include "WireCellUtil/Testing.h"

#include <boost/pipeline.hpp>
#include <iostream>

using namespace std;
using namespace boost::pipeline;

void test_queue()
{
    queue<int> q;
    queue_front<int> qf(q);
    queue_back<int> qb(q);

    Assert(qf.is_closed() == false);

    qb.push(1);
    qb.push(2);
    qb.push(3);

    int input = 0;

    qf.wait_pull(input);
    Assert(input == 1);

    qf.wait_pull(input);
    Assert(input == 2);

    qb.close();

    qf.wait_pull(input);
    Assert(input == 3);

    Assert(qf.is_closed());
}

void consume1(int x) { cerr << "consume1(" << x << ")" << endl; }
void consume2(queue_front<int>& qf)
{
    int i;
    qf.wait_pull(i);
    cerr << "consume2(" << i << ")" << endl;
}
int consume3(int x)
{
    return 0;
    cerr << "consume3(" << x << ")" << endl;
}
int consume4(queue_front<int>& qf)
{
    int i;
    qf.wait_pull(i);
    cerr << "consume4(" << i << ")" << endl;
    return 0;
}

void test_to()
{
    std::vector<int> input{0, 1, 2, 3};

    thread_pool pool(4);

    auto exec1 = (from(input) | consume1).run(pool);
    auto exec2 = (from(input) | consume2).run(pool);
    auto exec3 = (from(input) | to(consume3)).run(pool);
    auto exec4 = (from(input) | to(consume4)).run(pool);

    exec1.wait();
    exec2.wait();
    exec3.wait();
    exec4.wait();
}

void test_tofunction()
{
    auto f_consume1 = std::function<void(int)>(consume1);
    auto f_consume2 = std::function<void(queue_front<int>&)>(consume2);
    auto f_consume3 = std::function<int(int)>(consume3);
    auto f_consume4 = std::function<int(queue_front<int>&)>(consume4);

    std::vector<int> input{0, 1, 2, 3};

    thread_pool pool(4);

    auto exec1 = (from(input) | f_consume1).run(pool);
    auto exec2 = (from(input) | f_consume2).run(pool);
    auto exec3 = (from(input) | to(f_consume3)).run(pool);
    auto exec4 = (from(input) | to(f_consume4)).run(pool);

    exec1.wait();
    exec2.wait();
    exec3.wait();
    exec4.wait();
}

int main()
{
    test_queue();
    test_to();
    test_tofunction();

    return 0;
}
