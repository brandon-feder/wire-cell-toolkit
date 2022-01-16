#include "WireCellUtil/DfpGraph.h"
#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/Persist.h"

#include <iostream>

using namespace std;

int main()
{
    using namespace WireCell;

    std::string json = R"(
{
"edges":[
    {
	"tail": {"type":"A","name":"a"},
	"head": {"type":"B","name":""}
    },
    {
	"tail": {"type":"A","name":"a"},
	"head": {"type":"B","name":"b"}
    },
    {
	"tail": {"type":"A","name":"a","port":1},
	"head": {"type":"B","name":"b","port":1}
    },
    {
	"tail": {"type":"A","name":"a","port":2},
	"head": {"type":"C","name":"c","port":1}
    },
    {
	"tail": {"type":"A","name":"a","port":2},
	"head": {"type":"D","name":"d","port":1}
    },
    {
	"tail": {"type":"C","name":"c","port":1},
	"head": {"type":"D","name":"d","port":2}
    }
]
}
)";
    auto cfg = Persist::loads(json);

    DfpGraph dfp;
    dfp.configure(cfg["edges"]);

    for (auto thc : dfp.connections()) {
        auto tail = get<0>(thc);
        auto head = get<1>(thc);
        auto conn = get<2>(thc);
        cerr << tail << " " << conn << " " << head << endl;
    }

    return 0;
}
