#include "WireCellUtil/Testing.h"

#include <vector>
#include <set>
#include <deque>

#include <iterator>   // std::back_inserter
#include <algorithm>  // std::set_difference
#include <iostream>
#include <memory>
#include <random>

using namespace std;

int main()
{
    // http://stackoverflow.com/a/10604500
    vector<int> items = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    set<int> die = {2, 4, 5, 42};
    set<int> die2;

    vector<int> result;
    set_difference(items.begin(), items.end(), die.begin(), die.end(), back_inserter(result));
    cout << "items: ";
    for (auto item : items) {
        cout << item << " ";
    }
    cout << endl;
    cout << "alive: ";
    for (auto alive : result) {
        cout << alive << " ";
    }
    cout << endl;

    vector<int> result2;
    set_difference(items.begin(), items.end(), die2.begin(), die2.end(), back_inserter(result2));

    Assert(items.size() == 10);
    Assert(die.size() == 4);
    Assert(result.size() == 7);

    Assert(die2.size() == 0);
    Assert(result2.size() == 10);

    {
        std::random_device rd;
        std::default_random_engine re(rd());
        std::uniform_real_distribution<> dist(0, 1000);

        typedef std::shared_ptr<int> Pint;
        deque<Pint> queue;
        const int nitems = 1000;
        for (int ind = 0; ind < nitems; ++ind) {
            queue.push_back(Pint(new int(dist(re))));
        }
        for (int ind = 0; ind < nitems; ++ind) {
            Pint front = queue.front();
            queue.pop_front();
            cerr << ind << ": popped:" << *front << " now with: " << queue.size() << " items" << endl;
        }
    }
}
