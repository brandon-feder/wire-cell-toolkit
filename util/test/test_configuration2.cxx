#include "WireCellUtil/Configuration.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Point.h"

#include <iostream>

using namespace WireCell;
using namespace std;

int main()
{
    string json = R"(
[
   {
      "data": {
         "step_size": 1,
         "tracks": [
            {
               "charge": -1,
               "ray": {
                  "head": {
                     "x": 100,
                     "y": 10,
                     "z": 10
                  },
                  "tail": {
                     "x": 10,
                     "y": 0,
                     "z": 0
                  }
               },
               "time": 10
            },
            {
               "charge": -2,
               "ray": {
                  "head": {
                     "x": 2,
                     "y": -100,
                     "z": 0
                  },
                  "tail": {
                     "x": 1,
                     "y": 0,
                     "z": 0
                  }
               },
               "time": 120
            },
            {
               "charge": -3,
               "ray": {
                  "head": {
                     "x": 11,
                     "y": -50,
                     "z": -30
                  },
                  "tail": {
                     "x": 130,
                     "y": 50,
                     "z": 50
                  }
               },
               "time": 99
            }
         ]
      },
      "type": "TrackDepos"
   }
]
)";

    Configuration cfg = Persist::loads(json);

    for (auto comp : cfg) {
        Assert(get<string>(comp, "type") == "TrackDepos");
        Configuration data = comp["data"];
        Assert(get<double>(data, "step_size") == 1.0);
        Configuration tracks = data["tracks"];
        for (auto track : tracks) {
            Assert(get<double>(track, "charge") < 0);
            Ray ray = get<Ray>(track, "ray");
            cerr << ray << endl;
        }
    }
    return 0;
}
