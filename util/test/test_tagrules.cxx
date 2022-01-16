/* Test TagRules

   This has some built-in testing data but can also be used to test
   from the command line.

   test_tagrules file.jsonnet tag1 tag2 ...

   The file (can be JSON also) must follow schema:
   [array
     {category1:
       match1a: tagset1a
       match1b: tagset1b
     },
     {category2:
       match2: tagset2
     },
   ]

 */

#include "WireCellUtil/TagRules.h"
#include "WireCellUtil/Persist.h"

#include <iostream>

using namespace std;
using namespace WireCell;

int main(int argc, char* argv[])
{
    std::string jtext = R"(
[
    {
        frame: {
            tagXY: ["tagX%d"%n, "tagY%d"%n],
            "is-tagged\\d": "was-tagged",
            ["is-tagged%d"%n]: "was-tagged%d"%n,
        },
        // Output traces are tagged based on a map from an
        // input to one or more output tags.
        trace: {
            intagA: "outtagA%d"%n,
            intagB: ["outtagB%d"%n, "outtagBB%d"%n],
        }
    } for n in std.range(0,5)
]
)";

    tagrules::tagset_t tags;
    Configuration cfg;
    if (argc > 1) {
        cfg = Persist::load(argv[1]);  // .jsonnet or .json file assumed
    }
    else {
        cfg = Persist::loads(jtext);
    }
    if (argc > 2) {
        for (int ind = 2; ind < argc; ++ind) {
            tags.insert(argv[ind]);
        }
    }
    else {
        tags.insert("tagXY");
        tags.insert("tagXY");  // intenional duplicate
        tags.insert("intagA");
        tags.insert("dontmatchme");  // a tag that shouldn't be matched
        tags.insert("is-tagged2");
    }

    int iport = 0;
    for (auto jport : cfg) {
        for (auto cat : jport.getMemberNames()) {
            auto jrules = jport[cat];
            auto rs = convert<tagrules::ruleset_t>(jrules);
            auto newtags = tagrules::transform(tags, rs);
            std::cout << "port:" << iport << ", categ:\"" << cat << "\": have tags:[";
            string comma = "";
            for (auto t : tags) {
                std::cout << comma << "\"" << t << "\"";
                comma = ", ";
            }
            std::cout << "], got tags: [";
            comma = "";
            for (auto t : newtags) {
                std::cout << comma << "\"" << t << "\"";
                comma = ", ";
            }
            std::cout << "], with rules:" << std::endl;
            std::cout << jrules << std::endl;
        }
        ++iport;
        std::cout << std::endl;
    }
    return 0;
}
