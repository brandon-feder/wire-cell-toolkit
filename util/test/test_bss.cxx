#include <boost/signals2.hpp>
#include <iostream>

struct CountOutLoud {
    int count;
    CountOutLoud(int c = 0)
      : count(c)
    {
    }

    void operator()()
    {
        std::cout << "count: " << count << std::endl;
        ++count;
    }
};

int main()
{
    // Signal with no arguments and a void return value
    boost::signals2::signal<void()> sig;

    CountOutLoud col;
    sig.connect(col);

    sig();
    sig();
    sig();
}
