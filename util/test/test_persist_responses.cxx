#include "WireCellUtil/Response.h"
#include "WireCellUtil/ExecMon.h"
#include <iostream>
using namespace std;

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        cerr << "This test requires an Wire Cell Field Response input file." << endl;
        return 0;
    }

    WireCell::ExecMon em("test_persist_responses");
    auto fr = WireCell::Response::Schema::load(argv[1]);
    em("loaded");

    em("loop");
    cerr << fr.planes.size() << " planes" << endl;
    for (auto plane : fr.planes) {
        cerr << "\t" << plane.planeid << ": " << plane.paths.size() << " paths" << endl;
        for (auto path : plane.paths) {
            cerr << "\t\t" << path.current.size() << " samples\n\t\t";
            for (auto c : path.current) {
                cerr << c << " ";
            }
            cerr << endl;
            break;
        }
    }
    cerr << em.summary() << endl;
    // with -ggdb3: size=133936K, res=105216K about the same with opt.
    return 0;
}
