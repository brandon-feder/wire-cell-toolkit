#include "WireCellUtil/Persist.h"

#include <random>
#include <iostream>
#include <functional>

using namespace std;
using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string name = argv[0];

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    auto uni = std::bind(distribution, generator);

    const int nbins = 500;
    const int narrays = 100;
    Json::Value jroot;
    for (int ind = 0; ind < narrays; ++ind) {
        Json::Value jarr;
        for (int ibin = 0; ibin < nbins; ++ibin) {
            auto number = uni();
            jarr.append(number);
        }
        jroot[ind] = jarr;
    }

    // dump non-pretty in plain text and bzip2'ed to see size diff
    Persist::dump(name + ".json", jroot);
    Persist::dump(name + "-ugly.json.bz2", jroot);

    // dump pretty, read back and redump so can do a diff
    Persist::dump(name + "-pretty.json", jroot, true);
    Persist::dump(name + "-pretty.json.bz2", jroot, true);

    Json::Value jroot2 = Persist::load(name + "-pretty.json");
    Json::Value jroot3 = Persist::load(name + "-pretty.json.bz2");

    Persist::dump(name + "-pretty2.json", jroot2, true);
    Persist::dump(name + "-pretty3.json", jroot3, true);
    return 0;
}
