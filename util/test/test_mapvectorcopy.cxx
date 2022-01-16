#include "WireCellUtil/Testing.h"

#include <cmath>
#include <map>
#include <vector>
#include <iostream>

typedef std::vector<float> sequence;
typedef std::map<int, sequence> enumerated;

using namespace std;

void mutate(enumerated& en)
{
    en[0][0] = 42.0;
    en[1][0] = 6.9;
}

bool equal(double a, double b)
{
    if (a == b) {
        return true;
    }

    return std::abs(a - b) / (a + b) < 0.0001;
}

int main()
{
    sequence vec{1.0, 2.0, 3.0};
    enumerated stuff;
    stuff[0] = vec;
    stuff[1] = {10.0, 20.0, 30.0};
    mutate(stuff);

    for (auto iv : stuff) {
        cerr << iv.first << ":";
        for (auto x : iv.second) {
            cerr << " " << x;
        }
        cerr << endl;
    }

    Assert(equal(stuff[0][0], 42.0));
    Assert(equal(stuff[1][0], 6.9));
    Assert(equal(vec[0], 1.0));

    return 0;
}
