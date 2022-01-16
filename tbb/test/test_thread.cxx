#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
void msleep(int msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }

using namespace std;

struct Fun {
    int n;
    Fun(int n)
      : n(n)
    {
    }
    int operator()(int x, bool dec)
    {
        while (x > 0) {
            int old_n = n;
            if (dec) {
                n = x;
            }
            stringstream msg;
            msg << this << " n=" << n << " (was:" << old_n << ") x=" << x << "\n";
            cerr << msg.str();
            --x;
            msleep(x);
        }
        return 0;
    }
};

int main()
{
    Fun f1(1);

    std::thread t1(f1, 42, true);
    std::thread t2(f1, 69, false);
    t1.join();
    t2.join();

    return 0;
}
