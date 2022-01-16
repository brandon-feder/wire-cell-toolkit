#include <algorithm>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>

using namespace std;

// a global variable
std::list<int> myList;

// a global instance of std::mutex to protect global variable
std::mutex myMutex;

void hello(int id, int max)
{
    std::lock_guard<std::mutex> guard(myMutex);
    for (int i = 0; i < max; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        cout << "\033[1;31mhello " << id << "\033[0m ";
    }
}
void world(int id, int max)
{
    std::lock_guard<std::mutex> guard(myMutex);
    for (int i = 0; i < max; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        cout << "world " << id << " ";
    }
}

int main()
{
    int max = 10;

    std::thread t1(hello, 1, max);
    std::thread t2(world, 1, max);
    std::thread t3(world, 2, max);

    t1.join();
    t2.join();
    t3.join();

    cout << "\n";

    return 0;
}