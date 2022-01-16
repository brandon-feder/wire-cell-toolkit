#include "WireCellUtil/Array.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace std;
using namespace WireCell;
using namespace WireCell::Array;

array_xxf my_great_array(ExecMon& em, int nrows, int ncols)
{
    std::stringstream ss;
    ss << "array(" << nrows << "," << ncols << ")";
    em(ss.str() + ": constructing");
    array_xxf arr = Eigen::ArrayXXf::Random(nrows, ncols);  // 197 ms opt, 646 ms debug.
    em(ss.str() + ": constructed");

    return arr;
}

array_xxf dup(const array_xxf& arr) { return arr; }

void test_copy(ExecMon& em)
{
    const int nrows = 3000;
    const int ncols = 10000;
    {
        array_xxf arr = my_great_array(em, nrows, ncols);
        em("array by value: returned");

        // This loop over 100 copies takes 19.513 s (debug), 1.715 s (opt)
        for (int ind = 0; ind < 100; ++ind) {
            array_xxf tmp = dup(arr);
        }
        em("array by value: 100 dups");
    }
    em("array by value: left scope");
}

void test_return(ExecMon& em)
{
    const int nrows = 3000;
    const int ncols = 10000;
    {
        auto arr = my_great_array(em, nrows, ncols);
        em("ret: to auto");
        Assert(arr.rows() == nrows);
    }
    em("ret: out of scope");
}

template <typename arrtype>
bool same(const arrtype& a1, const arrtype& a2, double eps = 1.0e-6)
{
    double n1 = a1.matrix().squaredNorm();
    double n2 = a2.matrix().squaredNorm();
    if (n1 == n2) {
        return true;
    }
    if (n2 == 0.0) {
        cerr << "norm2=0, norm1=" << n2 << endl;
        return false;
    }
    double diff = std::abs(1 - n1 / n2);
    if (diff > eps) {
        cerr << "differ: " << diff << " n1=" << n1 << " n2=" << n2 << endl;
    }
    return (diff <= eps);
}

void test_partial(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    auto arr = my_great_array(em, nrows, ncols);
    em("test_partial: make array");

    auto spec = dft(arr);
    auto spec_rc = dft_rc(arr);
    auto spec_cc = dft_cc(spec_rc);
    em("test_partial: forward");

    auto arr2 = idft(spec);
    auto arr2_cc = idft_cc(spec_cc);
    auto arr2_cr = idft_cr(arr2_cc);
    em("test_partial: reverse");

    Assert(same(spec, spec_cc));
    Assert(same(arr, arr2));
    Assert(same(arr2, arr2_cr));

    const int nrounds = 100;

    for (int count = 0; count < nrounds; ++count) {
        auto spec = dft(arr);
        auto orig = idft(spec);
    }
    em("test_partial: direct round trip");
    for (int count = 0; count < nrounds; ++count) {
        auto spec_rc = dft_rc(arr);
        auto spec_cc = dft_cc(spec_rc);
        auto arr2_cc = idft_cc(spec_cc);
        auto arr2_cr = idft_cr(arr2_cc);
    }
    em("test_partial: partial round trip");
}

void test_dft(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    const int nrounds = 100;

    auto arr = my_great_array(em, nrows, ncols);
    em("dft: make array");
    for (int count = 0; count < nrounds; ++count) {
        auto spec = dft(arr);
        auto orig = idft(spec);
    }
    em("dft with floats");
#ifdef WCT_HACK_FOR_FFTW_NO_SP
    for (int count = 0; count < nrounds; ++count) {
        auto spec = dftd(arr);
        auto orig = idftd(spec);
    }
    em("dft up/down cast through doubles");
#endif
}

void test_deconv(ExecMon& em)
{
    const int nrows = 300;
    const int ncols = 1000;

    em("deconv: start");
    auto arr = my_great_array(em, nrows, ncols);
    em("deconv: got array");
    array_xxc filt = Eigen::ArrayXXcf::Zero(nrows, ncols) + 1.0;
    em("deconv: got filter");
    auto deco = deconv(arr, filt);
    em("deconv: done");  // 38ms opt, 274 debug

    array_xxf diff = arr - deco;
    em("deconv: diff");

    double norm = diff.matrix().norm();
    em("deconv: norm");
    cerr << "got norm of diff " << norm << endl;
    Assert(norm < 0.001);
}

void test_division(ExecMon& em)
{
    array_xxf arr1(3, 2), arr2(3, 2), arr3(3, 2);
    arr1 << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0;
    arr2 << 0.0, 0.5, 0.0, 2.0, 0.0, -5.0;
    arr3 = arr1 / arr2;
    cerr << "arr3 before NaN zeroing\n" << arr3 << endl;

    for (int irow = 0; irow < arr3.rows(); ++irow) {
        for (int icol = 0; icol < arr3.cols(); ++icol) {
            float val = arr3(irow, icol);
            if (std::isnan(val)) {
                arr3(irow, icol) = -0.0;
            }
            if (std::isinf(val)) {
                arr3(irow, icol) = 0.0;
            }
        }
    }
    cerr << "arr3 after NaN zeroing\n" << arr3 << endl;
}

void test_division_complex(ExecMon& em)
{
    array_xxc arr1(3, 2), arr2(3, 2), arr3(3, 2);
    arr1 << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0;
    arr2 << 0.0, 0.5, 0.0, 2.0, 0.0, -5.0;
    arr3 = arr1 / arr2;
    cerr << "arr3 before NaN zeroing\n" << arr3 << endl;

    for (int irow = 0; irow < arr3.rows(); ++irow) {
        for (int icol = 0; icol < arr3.cols(); ++icol) {
            float val = abs(arr3(irow, icol));
            if (std::isnan(val)) {
                arr3(irow, icol) = -0.0;
            }
            if (std::isinf(val)) {
                arr3(irow, icol) = 0.0;
            }
        }
    }
    cerr << "arr3 after NaN zeroing\n" << arr3 << endl;
}

int main()
{
    WireCell::ExecMon em;

    test_partial(em);
    test_copy(em);
    test_return(em);
    test_dft(em);
    test_deconv(em);
    test_division(em);
    test_division_complex(em);

    em("the end");
    cerr << em.summary() << endl;
    return 0;
}
