
#include "WireCellUtil/Iterator.h"
#include "WireCellUtil/IteratorBase.h"
#include "WireCellUtil/Testing.h"
#include <boost/range.hpp>

#include <boost/function.hpp>

#include <vector>
#include <iostream>
#include <algorithm>  // copy_if
#include <iterator>   // back_inserter
#include <memory>     // shared_ptr
#include <vector>
#include <cmath>

using namespace std;
using namespace WireCell;

// This is the base/interface data type that will be iterated over.
struct IMyClass {
    virtual ~IMyClass() {}
    virtual int get_i() const = 0;
    virtual float get_f() const = 0;
};

// This is a concrete imp of that interface and may reside deep inside
// some private scope.
struct MyClass : public IMyClass {
    int m_i;
    float m_f;

    MyClass(int i = 0, float f = 0.0)
      : m_i(i)
      , m_f(f)
    {
    }
    virtual ~MyClass() {}
    virtual int get_i() const { return m_i; }
    virtual float get_f() const { return m_f; }
};

// What I expose to my clients.  Ie, something that looks lika a
// std::vector<const IMyClass*>::iterator.
typedef Iterator<const IMyClass*> my_iterator;

// Begin/end pairs passed to and by clients.
typedef std::pair<my_iterator, my_iterator> my_range;

// What I and all other implementers of my interface use as an
// abstract base class for the iterator of the base data type.  The
// template parameter must match the one used to define my_iterator.
typedef IteratorBase<const IMyClass*> my_base_iterator;

// In the end, the concrete imp that exposes iterators to IMyClass
// needs to store implementation-specific data types and this store is
// here.  It need not be exposed to anyone but the concrete-imp.  It's
// non-const since I own it and control when it's finalized.  I may
// also delete it one day.
typedef vector<MyClass*> MyStoreType;

// Since the above store is really just an STL vector the
// implementation need not explicitly write matching concrete
// iterator, but can use the STL adapter.
typedef IteratorAdapter<MyStoreType::iterator, my_base_iterator> my_adapted_iterator;

// Selectors
typedef boost::function<bool(const IMyClass*)> my_selector;

struct SelectInt {
    int target;
    SelectInt(int want)
      : target(want)
    {
    }
    bool operator()(const IMyClass* obj) { return obj->get_i() == target; }
};

struct SelectInRange {
    float minval, maxval;
    SelectInRange(float min, float max)
      : minval(min)
      , maxval(max)
    {
    }
    bool operator()(const IMyClass* obj) { return minval <= obj->get_f() && obj->get_f() < maxval; }
};

SelectInt get1 = SelectInt(1);
my_selector get10 = SelectInt(10);
SelectInRange just_right = SelectInRange(5, 10);

my_range get_data(MyStoreType& store)
{
    return my_range(my_adapted_iterator(store.begin()), my_adapted_iterator(store.end()));
}

int main()
{
    // This would be deep in side some custom class.
    MyStoreType store;
    store.push_back(new MyClass(0, 0.0));
    store.push_back(new MyClass(1, 42.));
    store.push_back(new MyClass(2, 6.9));

    MyClass* first = *(store.begin());
    AssertMsg(first, "No first");
    AssertMsg(first->get_i() == 0 && first->get_f() == 0.0, "first ain't first");

    // This would be an element of the base class's interface which
    // the custom class implements.  It returns generic iterators on
    // the base/interface data type.  These iterators may be copied
    // without slicing.
    my_range r = get_data(store);

    // Finally, here is some client of the interface using the data
    // born deep inside the custom class and accessing it only via
    // interfaces.
    for (my_iterator it = boost::begin(r); it != boost::end(r); ++it) {
        // make a temp for syntactic convenience/clarity
        const IMyClass* myptr = *it;
        const IMyClass& myobj = *myptr;
        cout << myobj.get_i() << " " << myobj.get_f() << endl;
    }

    vector<const IMyClass*> res(r.first, r.second);
    AssertMsg(res.size(), "range constructor failed.");
    res.clear();

    copy(r.first, r.second, back_inserter(res));
    AssertMsg(res.size(), "copy failed.");
    res.clear();

    copy_if(boost::begin(r), boost::end(r), back_inserter(res), get1);
    cerr << "Got: " << res.size() << endl;
    AssertMsg(1 == res.size(), "Failed to get1");
    AssertMsg(res[0]->get_f() == 42., "Got wrong1");
    res.clear();

    copy_if(boost::begin(r), boost::end(r), back_inserter(res), get10);
    AssertMsg(0 == res.size(), "Got get10 but should not.");
    res.clear();

    copy_if(boost::begin(r), boost::end(r), back_inserter(res), just_right);
    AssertMsg(1 == res.size(), "Failed to get just_right");
    cerr << "Got: " << res[0]->get_i() << " " << res[0]->get_f() << endl;
    AssertMsg(res[0]->get_i() == 2 && fabs(res[0]->get_f() - 6.9) < 1e-6, "Got just_wrong value");
    res.clear();

    return 0;
}
