#include <boost/any.hpp>
#include <memory>
#include <iostream>

using namespace std;

struct Base {
    virtual ~Base() {}
    int b;
};
struct Foo : public Base {
    virtual ~Foo() {}
    int a;
};

enum DFPMeta { notdfp, eos, restart };

int main()
{
    boost::any aobj;
    {
        aobj = std::make_shared<Foo>();
    }
    auto base = boost::any_cast<std::shared_ptr<Foo> >(aobj);

    if (!base) {
        cerr << "base is null\n";
    }
    else {
        cerr << "base is not null\n";
    }

    boost::any what = eos;
    if (eos == boost::any_cast<DFPMeta>(what)) {
        cerr << "Got eos from eos\n";
    }

    DFPMeta dfpm;
    try {
        dfpm = boost::any_cast<DFPMeta>(aobj);
    }
    catch (boost::bad_any_cast &e) {
        std::cerr << "aobj not a DFPMeta: " << e.what() << '\n';
        dfpm = notdfp;
    }
    cerr << "DFPMeta = " << dfpm << "\n";

    return 0;
}
