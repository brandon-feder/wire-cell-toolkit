#include <iostream>
#include <functional>
#include <random>

typedef std::function<int()> intsrc_t;

struct MyIntFunc {
    int number = 0;
    MyIntFunc(int n = 0)
      : number(n)
    {
    }
    int operator()() { return number; }
};

intsrc_t return_a_func()
{
    MyIntFunc mif(42);
    return mif;
}

void take_a_func(intsrc_t f) { std::cerr << f() << std::endl; }

std::default_random_engine make_a_generator()
{
    std::default_random_engine generator(42);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::cerr << "Made generator with " << distribution(generator) << std::endl;
    return generator;
}
std::default_random_engine take_a_generator(std::default_random_engine generator)
{
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::cerr << "Take generator with " << distribution(generator) << std::endl;
    return generator;
}

int main()
{
    MyIntFunc f(666);
    take_a_func(f);

    intsrc_t f2 = return_a_func();
    take_a_func(f2);

    std::default_random_engine generator = make_a_generator();
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    std::cerr << "Used generator with " << distribution(generator) << std::endl;
    generator = take_a_generator(generator);
    std::cerr << "Copy generator with " << distribution(generator) << std::endl;
    generator = take_a_generator(generator);
    std::cerr << "Copy generator with " << distribution(generator) << std::endl;
}
