#include <iostream>
#include <typeinfo>
using namespace std;

struct Base {
    virtual ~Base() {}
    virtual void any() = 0;
};
struct A : public Base {
    virtual ~A() {}

    void a() { cerr << "Hi (a)\n"; }

    virtual void any() { cerr << "Hi from any (a)\n"; }
};
struct B : public Base {
    virtual ~B() {}
    void b() { cerr << "Hi (b)\n"; }

    virtual void any() { cerr << "Hi from any (b)\n"; }
};

Base* make_a() { return new A; }
Base* make_b() { return new B; }

int main()
{
    auto *a = make_a(), *b = make_b();
    a->any();
    b->any();

    typedef decltype(*a) MyA;
    cerr << typeid(MyA).name() << endl;

    // decltype(*a)* aa = dynamic_cast<decltype(*a)*>(a);

    // aa->any();
    // aa->a();

    return 0;
}
