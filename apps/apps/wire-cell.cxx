/** CLI to WireCell::Main. */

#include "WireCellApps/Main.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

using namespace WireCell;
using namespace std;

int main(int argc, char* argv[])
{
    Main m;
    int rc = 0;

    try {
        rc = m.cmdline(argc, argv);
        if (rc) {
            return rc;
        }
        m.initialize();
        m();
    }
    catch (Exception& e) {
        cerr << errstr(e) << endl;
        return 1;
    }
    return 0;
}
