#include "WireCellUtil/BufferedHistogram2D.h"
#include "WireCellUtil/Testing.h"

#include <vector>
#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    BufferedHistogram2D hist;
    Assert(hist.popx().size() == 0);
    Assert(false == hist.fill(-1, -1, 0));
    Assert(hist.xmin() == 0.0);
    Assert(hist.ymin() == 0.0);

    cerr << "mins: " << hist.xmin() << " " << hist.ymin() << endl;

    Assert(hist.fill(0.5, 3.5));
    vector<double> dat = hist.popx();

    cerr << "dat size " << dat.size() << endl;

    Assert(dat.size() == 4);
    Assert(dat[3] == 1.0);
    Assert(hist.xmin() == 1.0);
    Assert(hist.ymin() == 0.0);
}
