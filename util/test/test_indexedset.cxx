#include "WireCellUtil/Testing.h"
#include "WireCellUtil/IndexedSet.h"

using namespace WireCell;

int main()
{
    IndexedSet<int> isi;
    AssertMsg(isi(42) == 0, "Failed to index");
    AssertMsg(isi(69) == 1, "Failed to index");
    AssertMsg(isi(42) == 0, "Failed to index");
    AssertMsg(isi(0) == 2, "Failed to index");
    AssertMsg(isi(0) == 2, "Failed to index");
    AssertMsg(isi(69) == 1, "Failed to index");

    AssertMsg(isi.collection.size() == 3, "Wrong number of stuff in the collection");

    return 0;
}
