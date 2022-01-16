// boost iterator facade look-ahead test

#include "WireCellUtil/GeneratorIter.h"

#include <iostream>
using namespace std;
using namespace WireCell;

template <typename Container>
struct EveryOther {
    typedef typename Container::value_type value_type;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    const_iterator it, other, end;

    EveryOther(const_iterator begin, const_iterator end)
      : it(begin)
      , other(begin)
      , end(end)
    {
        if (other != end) ++other;
        if (other != end) ++other;
        cerr << "Creating EveryOther " << std::distance(begin, end) << endl;
    }
    value_type operator()()
    {
        value_type ret{};
        if (*this) {
            ret = *it + *other;
            ++it;
            ++other;
        }
        cerr << "operator() returns at " << std::distance(it, end) << endl;
        return ret;
    }
    operator bool() const
    {
        bool exhausted = false;
        if (end - it < 2) {
            exhausted = true;
        }
        cerr << "Are we exhausted? " << exhausted << endl;
        return !exhausted;
    }
    bool operator==(const EveryOther& rhs) const
    {
        if (this == &rhs) return true;
        return it == rhs.it;
    }
};

#include <vector>

typedef std::vector<int> IntVector;
typedef EveryOther<IntVector> EOIV;
typedef GeneratorIter<EOIV, int> eo_iterator;
typedef std::pair<eo_iterator, eo_iterator> eo_range;

eo_range make_every_other(IntVector::iterator begin, IntVector::iterator end)
{
    return eo_range(eo_iterator(EOIV(begin, end)), eo_iterator(EOIV(end, end)));
}

int main()
{
    IntVector vi;
    vi.push_back(0);
    vi.push_back(1);
    vi.push_back(2);
    vi.push_back(3);
    vi.push_back(4);
    vi.push_back(5);
    vi.push_back(6);

    auto its = make_every_other(vi.begin(), vi.end());

    for (auto it = its.first; it != its.second; ++it) {
        cout << *it << endl;
    }
    return 0;
}
