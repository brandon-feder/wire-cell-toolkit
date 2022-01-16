#include <boost/signals2.hpp>

#include <iostream>
#include <memory>

// http://stackoverflow.com/a/20429450

typedef boost::signals2::signal<int()> IntSig;
typedef std::shared_ptr<IntSig> IntSigPtr;
typedef IntSig::slot_type IntSlot;

using namespace std;

struct A {
    int x;
    double dd;
    A(int n, double d)
      : x(n)
      , dd(d)
    {
    }
    int operator()()
    {
        cout << "A @ " << x << " " << dd << endl;
        dd *= x;
        return x++;
    }
};

struct B {
    int y;
    double s;
    IntSigPtr sig;

    B(double scale)
      : y(0)
      , s(scale)
      , sig(new IntSig)
    {
    }
    void connect(const IntSlot& slot) { sig->connect(slot); }
    int operator()()
    {
        y = s * *(*sig)();
        cout << "B @ " << y << endl;
        return y;
    }
};

void test_simple()
{
    A a(42, 1.0);
    B b(1.0);
    b.connect(a);
    B b2(2.0);
    b2.connect(b);
    B b3(3.0);
    b3.connect(b2);

    b3();
    b3();
}

struct MyData {
    typedef std::shared_ptr<MyData> pointer;
    int x;
    MyData(int n = 0)
      : x(n)
    {
    }
    int next() { return ++x; }
    int get() { return x; }

    typedef boost::signals2::signal<pointer()> source_signal;
    typedef typename source_signal::slot_type source_slot;
};

struct SlotA {
    MyData::pointer data;
    SlotA()
      : data(new MyData)
    {
    }
    MyData::pointer operator()()
    {
        data->next();
        return data;
    }
};

struct SigA {
    SigA() {}

    MyData::pointer operator()() { return *m_input(); }

    void connect(const MyData::source_slot& s) { m_input.connect(s); }
    MyData::source_signal m_input;
};

void test_isignal()
{
    SlotA slota;
    SigA siga, sigaa;
    siga.connect(slota);
    sigaa.connect(boost::ref(siga));
    cout << sigaa()->get() << endl;
    ;
    cout << sigaa()->get() << endl;
    ;
    cout << sigaa()->get() << endl;
    ;
}

int main()
{
    test_simple();
    test_isignal();
    return 0;
}
