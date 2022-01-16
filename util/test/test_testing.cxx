/** Test the WireCellUtil/Testing helpers.
 */

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"

#include <iostream>

using namespace WireCell;
using namespace std;

int main(int argc, char** argv)
{
    AssertMsg(argc == 1, "In general, tests should not be called with arguments.");
    Assert(argc == 1);

    try {  // normally, one wouldn't catch assertion errors in tests.
        AssertMsg(false, "this assert should be caught");
    }
    catch (AssertionError& e) {
        cerr << "Caught:\n" << errstr(e) << endl;
    }
    return 0;
}
