#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

using namespace std;

struct IName {
    virtual std::string name() const = 0;
    virtual ~IName() {}
};

template <typename T>
struct INameT : public IName {
    virtual std::string name() const { return typeid(T).name(); }
    virtual ~INameT() {}
};

struct A : public INameT<int>, public INameT<float> {
    virtual ~A() {}
};

int main()
{
    A a;

    cerr << a.INameT<int>::name() << endl;
    cerr << a.INameT<float>::name() << endl;

    // ambiguous, fails to compile.
    // cerr << a.name() << endl;

    INameT<int>* ai = &a;
    cerr << ai->name() << endl;

    INameT<float>* af = &a;
    cerr << af->name() << endl;

    cerr << "Via bases" << endl;
    vector<IName*> named{ai, af};
    for (auto n : named) {
        cerr << n->name() << endl;
    }

    return 0;
}
