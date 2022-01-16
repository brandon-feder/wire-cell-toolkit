#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Testing.h"

#include <cstdlib>
#include <string>
#include <iostream>

using namespace WireCell;

int main()
{
    Assert(Persist::exists("/etc"));
    Assert(Persist::exists("/etc/hosts"));

    setenv("WIRECELL_PATH", "/etc:/usr:/var", 1);
    std::string etchosts = Persist::resolve("hosts");
    // std::cerr << etchosts << std::endl;
    Assert(etchosts == "/etc/hosts");

    std::string dne = Persist::resolve("this_file-really_should-not_exist");
    Assert(dne.empty());

    return 0;
}
