/**
 * The idea is that there are objects of a base interface IComponent
 * which may also implement some other interface, here IPort.  The
 * objects are initially known through IComponent and a templated
 * function, transfer(), must be called on these objects but through a
 * middle sublcass which contains some needed type info, here Port<T>.
 *
 * The solution is to create a context (CallTransfer and
 * CallTransferT<> structs) which knows the type T and then register a
 * number of instances of this context using the type_info::name() of
 * the instantiated T.
 *
 * Then, the base interface IPort has a pure abstract method to return
 * the type_info::name() supplied by the subclass.  This is used to
 * look up the corresponding context which takes the base IPort
 * objects and dynamic_cast's them to the needed subclass.
 */

#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Interface.h"
#include <iostream>
#include <typeinfo>
#include <map>
#include <string>

using namespace std;

struct IPort {
    virtual ~IPort(){};
    virtual std::string port_type_name() const = 0;
};

template <typename T>
struct Port : public IPort {
    typedef T port_type;
    virtual ~Port() {}
    virtual std::string port_type_name() const { return typeid(port_type).name(); }
    virtual bool put(const port_type& in) { return false; }
    virtual bool get(port_type& out) const { return false; }
    virtual T make() const { return 0; }
};

struct SubI : public Port<int> {
    virtual ~SubI() {}
    virtual bool get(port_type& out) const
    {
        out = 42;
        cerr << "SubI::get(" << out << ")" << endl;
        return true;
    }
};
struct SubF : public Port<float> {
    virtual ~SubF() {}
    virtual bool get(port_type& out) const
    {
        out = 6.9;
        cerr << "SubF::get(" << out << ")" << endl;
        return true;
    }
};

struct SubFIn : public Port<float> {
    virtual ~SubFIn() {}
    virtual bool put(const port_type& in)
    {
        cerr << "SubFIn::put(" << in << ")" << endl;
        return true;
    }
    virtual bool get(port_type& out) const
    {
        cerr << "SubIn::get() can not provide data" << endl;
        return true;
    }
};

struct SubFOut : public Port<float> {
    virtual ~SubFOut() {}
    virtual bool put(const port_type& in)
    {
        cerr << "SubFOut::put(" << in << ") can not accept data" << endl;
        return false;
    }
    virtual bool get(port_type& out) const
    {
        out = 6.9;
        cerr << "SubFOut::get(" << out << ")" << endl;
        return true;
    }
};

template <typename A, typename B>
bool transfer(const A& a, B& b)
{
    cerr << "transfer: " << typeid(A).name() << "<->" << typeid(a).name() << " " << typeid(B).name() << "<->"
         << typeid(b).name() << endl;

    if (a.port_type_name() != b.port_type_name()) {
        cerr << "Port type mismatch: " << a.port_type_name() << " != " << b.port_type_name() << endl;
        return false;
    }
    typename A::port_type dat;
    if (!a.get(dat)) {
        cerr << "Failed to get output of type " << a.port_type_name() << endl;
        return false;
    }
    if (!b.put(dat)) {
        cerr << "Failed to put input of type " << b.port_type_name() << endl;
        return false;
    }
    return true;
}

struct CallTransfer {
    virtual ~CallTransfer() {}
    virtual bool call(const IPort& a, IPort& b) = 0;
};

template <typename T>
struct CallTransferT : public CallTransfer {
    typedef T port_type;
    virtual ~CallTransferT() {}
    std::string port_type_name() const { return typeid(port_type).name(); }
    bool call(const IPort& a, IPort& b)
    {
        const Port<port_type>* pa = dynamic_cast<const Port<port_type>*>(&a);
        Port<port_type>* pb = dynamic_cast<Port<port_type>*>(&b);
        return transfer(*pa, *pb);
    }
};

static map<string, CallTransfer*> callers;

template <typename T>
void register_caller()
{
    CallTransferT<T>* ct = new CallTransferT<T>;
    callers[ct->port_type_name()] = ct;
}

CallTransfer* get_caller(const IPort& comp)
{
    const IPort* port = dynamic_cast<const IPort*>(&comp);
    if (!port) {
        return nullptr;
    }
    return callers[port->port_type_name()];
}

int main()
{
    IPort* si = new SubI;
    IPort* sf = new SubF;
    IPort* sfi = new SubFIn;
    IPort* sfo = new SubFOut;

    cout << "typeid(si) = " << typeid(si).name() << endl;
    cout << "typeid(*si) = " << typeid(*si).name() << endl;
    cout << "typeid(sf) = " << typeid(sf).name() << endl;
    cout << "typeid(*sf) = " << typeid(*sf).name() << endl;
    cout << "typeid(sfo) = " << typeid(sfo).name() << endl;
    cout << "typeid(*sfo) = " << typeid(*sfo).name() << endl;
    cout << "typeid(sfi) = " << typeid(sfi).name() << endl;
    cout << "typeid(*sfi) = " << typeid(*sfi).name() << endl;

    cout << "typeid(SubI::port_type) = " << typeid(SubI::port_type).name() << endl;
    cout << "typeid(SubF::port_type) = " << typeid(SubF::port_type).name() << endl;
    cout << "typeid(SubFOut::port_type) = " << typeid(SubFOut::port_type).name() << endl;
    cout << "typeid(SubFIn::port_type) = " << typeid(SubFIn::port_type).name() << endl;

    cout << "si->type_name() = " << si->port_type_name() << endl;
    cout << "sf->type_name() = " << sf->port_type_name() << endl;
    cout << "sfo->type_name() = " << sfo->port_type_name() << endl;
    cout << "sfi->type_name() = " << sfi->port_type_name() << endl;

    // given just Port*, how to know what to dynamic_cast to???
    Port<float>* bfi = dynamic_cast<Port<float>*>(sfi);
    Port<float>* bfo = dynamic_cast<Port<float>*>(sfo);
    Assert(bfi);
    Assert(bfo);
    Assert(transfer(*bfo, *bfi));

    // okay, like this:
    register_caller<int>();
    register_caller<float>();

    IPort* ci = new SubFIn;
    IPort* co = new SubFOut;

    CallTransfer* ct = get_caller(*ci);
    Assert(ct);
    Assert(ct->call(*co, *ci));

    return 0;
}
