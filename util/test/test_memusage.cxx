#include "WireCellUtil/MemUsage.h"

#include <iostream>
#include <sstream>
#include <vector>
//#include <unistd.h>

using namespace std;
using namespace WireCell;

int* suck(MemUsage& mu, long siz)
{
    int* buf = new int[siz * 1024];
    // sleep(1);
    {
        stringstream ss;
        ss << "newed " << siz << "K";
        cout << ss.str() << " " << mu(ss.str()) << endl;
    }

    for (int ind = 0; ind < siz; ++ind) {
        buf[ind] = 0;
    }
    {
        stringstream ss;
        ss << "accessed " << siz << "K";
        cout << ss.str() << " " << mu(ss.str()) << endl;
    }

    return buf;
}

void blow(MemUsage& mu, int siz, int* buf)
{
    delete[] buf;
    // sleep(1);
    stringstream ss;
    ss << "freed " << siz << "K";
    cout << ss.str() << " " << mu(ss.str()) << endl;
}

int main()
{
    cout << "current size " << memusage_size() << endl;
    cout << "current resident " << memusage_resident() << endl;
    cout << "current shared " << memusage_shared() << endl;

    MemUsage mu;

    long sizes[] = {10, 100, 1000, 10000, 100000, 1000000, -1};
    vector<int*> bufs;

    int ind = 0;
    for (; sizes[ind] > 0; ++ind) {
        int* buf = suck(mu, sizes[ind]);
        bufs.push_back(buf);
    }

    for (--ind; ind >= 0; --ind) {
        blow(mu, sizes[ind], bufs[ind]);
    }

    cout << "Summary:" << endl;
    cout << mu.summary() << endl;
}
