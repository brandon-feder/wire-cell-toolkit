/** Test the WireCellUtil/Testing helpers.
 */

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/ExecMon.h"

#include <unistd.h>  // fixme: maybe a more modern sleep() is preferable?
#include <iostream>

using namespace WireCell;
using namespace std;

int main(int argc, char** argv)
{
    ExecMon em("test_testing");
    cout << em("sleeping") << endl;
    ;
    sleep(1);
    cout << em("awake") << endl;

    cout << em.summary() << endl;
    return 0;
}
