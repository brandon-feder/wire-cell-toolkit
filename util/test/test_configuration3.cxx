#include "WireCellUtil/Configuration.h"
#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;
using namespace WireCell;

int main()
{
    Configuration cfg;
    cfg["chirp"] = "bad";
    cfg["bad"] = "bad";

    Configuration top;
    top["maskmap"] = cfg;

    // void OmnibusNoiseFilter::configure(const WireCell::Configuration& top)

    auto jmm = top["maskmap"];

    std::unordered_map<std::string, std::string> mm;
    for (auto name : jmm.getMemberNames()) {
        mm[name] = jmm[name].asString();
        cerr << name << " " << mm[name] << endl;
    }

    auto newthing = top["maskmap"]["adc"];
    // auto newthing = cfg["adc"];
    if (newthing.isNull()) {
        cerr << "dunno adc\n";
    }

    cerr << top << endl;
}
