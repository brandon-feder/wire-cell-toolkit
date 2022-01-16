#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    string json = R"(
{
"my_int": 1,
"my_float": 6.9,
"my_string": "hello",
"my_struct": { "x": 1, "y": 2, "z": 3 },
"my_array" : [ "one", "two", "three" ],
"my_array_dict" : [ {"a":1, "b":2}, {"a":10, "b":20}],
"data1" : { "data2": { "a":1, "b":2, "c":3 } }
}
)";
    string extra_json = R"(
{
"my_int" : 2,
"data1" : { "data2": { "d":4 } },
"data3": { "data4" : 4 }
}
)";

    Configuration cfg = Persist::loads(json);
    Configuration extra_cfg = Persist::loads(extra_json);

    Assert(get(cfg, "my_int", 0) == 1);
    Assert(get(cfg, "my_float", 0.0) == 6.9);

    Assert(string("hello") == convert<std::string>(cfg["my_string"]));

    cerr << "my_string=" << get(cfg, "my_string", string("")) << endl;
    Assert(get(cfg, "my_string", string("")) == string("hello"));

    put(cfg, "a.b.c", 42);
    cerr << cfg << endl;
    int n42get = get(cfg, "a.b.c", 0);
    Assert(42 == n42get);
    int n42ind = cfg["a"]["b"]["c"].asInt();
    Assert(42 == n42ind);

    Assert(get<int>(cfg, "my_struct.x") == 1);

    auto nums = get<vector<string> >(cfg, "my_array");
    for (auto anum : nums) {
        cerr << anum << endl;
    }

    Configuration a = branch(cfg, "data1.data2.a");
    Assert(convert<int>(a) == 1);

    Configuration other;
    update(other, cfg);
    Configuration last = update(other, extra_cfg);
    cerr << "other:\n" << other << endl;
    cerr << "last:\n" << last << endl;
    Assert(last["a"]["b"]["c"] == 42);
    Assert(last["data3"]["data4"] == 4);

    return 0;
}
