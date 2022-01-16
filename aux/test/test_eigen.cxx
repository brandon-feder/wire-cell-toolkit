#include "WireCellAux/Util.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

using namespace WireCell;

void test_translation()
{
    typedef float TestType;

    // Eigen::Array<TestType, Eigen::Dynamic, Eigen::Dynamic> iarr =
    // Eigen::Array<TestType, Eigen::Dynamic, Eigen::Dynamic>::Random(2, 3);
    Eigen::Array<TestType, 2, 3> iarr;
    iarr << 1, 2, 3, 4, 5, 6;
    std::cout << iarr << "\n";

    auto iten = Aux::eigen_array_to_itensor<TestType>(iarr);
    std::cout << Aux::dump<TestType>(iten) << "\n";

    auto oarr = Aux::itensor_to_eigen_array<TestType>(iten);
    std::cout << oarr << "\n";
    Assert(iarr.isApprox(oarr));

    // Eigen::Map copies?
    // oarr(0,0) = 42;
    // std::cout << oarr << "\n";
    // std::cout << dump<TestType>(iten) << "\n";
}
void test_speed(const int nloop = 1)
{
    typedef float TestType;

    Eigen::Array<TestType, Eigen::Dynamic, Eigen::Dynamic> iarr =
        Eigen::Array<TestType, Eigen::Dynamic, Eigen::Dynamic>::Random(6000, 800);
    auto iten = Aux::eigen_array_to_itensor<TestType>(iarr);
    auto oarr = Aux::itensor_to_eigen_array<TestType>(iten);
    Assert(iarr.isApprox(oarr));

    auto t1 = Clock::now();
    for (int i = 0; i < nloop; ++i) {
        iten = Aux::eigen_array_to_itensor<TestType>(iarr);
    }
    auto t2 = Clock::now();
    std::cout << "Eigen -> TensorSet: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / (float) nloop << " ms/loop"
              << std::endl;

    t1 = Clock::now();
    for (int i = 0; i < nloop; ++i) {
        oarr = Aux::itensor_to_eigen_array<TestType>(iten);
    }
    t2 = Clock::now();
    std::cout << "TensorSet -> Eigen: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / (float) nloop << " ms/loop"
              << std::endl;
}

int main()
{
    test_translation();
    test_speed(100);
    return 0;
}