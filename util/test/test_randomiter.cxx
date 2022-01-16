#include "WireCellUtil/RandomIter.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <vector>

using namespace WireCell;

int main()
{
    typedef int value;
    typedef std::vector<value> container;

    container array(4);
    array[0] = 1;
    array[1] = 2;
    array[2] = 3;
    array[3] = 4;

    typedef RandomIter<container, value> iterator;

    iterator beg(array), end(true, array);
    for (iterator it = beg; it != end; ++it) {
        std::cout << *it << std::endl;
    }

    beg += 2;
    std::cout << *beg << std::endl;
    Assert(3 == *beg);
    beg -= 2;
    std::cout << *beg << std::endl;
    Assert(1 == *beg);
    return 0;
}
