#include "WireCellAux/DftTools.h"

using namespace WireCell;
using namespace WireCell::Aux;

/*
  Big fat warning to future me: Passing by reference means the input
  array may carry the .IsRowMajor optimization for implementing
  transpose().  An extra copy would remove that complication but this
  interface tries to keep it.
 */

using ROWM = Eigen::Array<Aux::complex_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using COLM = Eigen::Array<Aux::complex_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

template<typename trans>
Aux::dft_array_t doit(const Aux::dft_array_t& arr, trans func)
{
    // Nominally, eigen storage memory is in column-major order
    const Aux::complex_t* in_data = arr.data();
    int ncols = arr.rows();
    int nrows = arr.cols();

    // except when it isn't
    bool flipped = arr.IsRowMajor;
    if (flipped) {
        ncols = arr.cols();
        nrows = arr.rows();
    }

    Aux::dft_vector_t out_vec(nrows*ncols);
    func(in_data, out_vec.data(), nrows, ncols);

    if (flipped) {
        return Eigen::Map<ROWM>(out_vec.data(), arr.rows(), arr.cols());
    }
    return Eigen::Map<COLM>(out_vec.data(), arr.rows(), arr.cols());

}

Aux::dft_array_t Aux::fwd(IDFT::pointer dft, const Aux::dft_array_t& arr)
{
    return doit(arr, [&](const complex_t* in_data,
                         complex_t* out_data,
                         int nrows, int ncols) {
        dft->fwd2d(in_data, out_data, nrows, ncols);
    });
}

Aux::dft_array_t Aux::inv(IDFT::pointer dft, const Aux::dft_array_t& arr)
{
    return doit(arr, [&](const complex_t* in_data,
                         complex_t* out_data,
                         int nrows, int ncols) {
        dft->inv2d(in_data, out_data, nrows, ncols);
    });
}

#include <iostream> // debug

template<typename trans>
Aux::dft_array_t doit1b(const Aux::dft_array_t& arr, int axis, trans func)
{
    // We must provide a flat array with storage order such with
    // logical axis-major ordering.
    const Aux::complex_t* in_data = arr.data();
    const int nrows = arr.rows(); // "logical"
    const int ncols = arr.cols(); // shape

    std::cerr << "nrows="<<nrows<<", ncols="<<ncols
              << ", axis="<<axis<<", IsRowMajor:"<<arr.IsRowMajor<<"\n";

    // If storage order matches "axis-major"
    if ( (axis == 1 and arr.IsRowMajor)
         or
         (axis == 0 and not arr.IsRowMajor) ) {
        Aux::dft_vector_t out_vec(nrows*ncols);
        func(in_data, out_vec.data(), ncols, nrows);
        if (arr.IsRowMajor) {
            // note, returning makes a copy and will perform an actual
            // storage order transpose.
            return Eigen::Map<ROWM>(out_vec.data(), nrows, ncols);
        }
        return Eigen::Map<COLM>(out_vec.data(), nrows, ncols);
    }
    
    // Either we have row-major and want column-major storage order or
    // vice versa.

    // Here, we must copy and not use "auto" to get actual storage
    // order transpose and avoid the IsRowMajor flip optimization.
    COLM flipped = arr.transpose();
    COLM got = doit1b(flipped, (axis+1)%2, func);
    return got.transpose();
}

// Implementation notes for fwd()/inv():
//
// - We make an initial copy to get rid of any potential IsRowMajor
//   optimization/confusion over storage order.  This suffers a copy
//   but we need to allocate return anyways.
//
// - We then have column-wise storage order but IDFT assumes row-wise
// - so we reverse (nrows, ncols) and meaning of axis.

Aux::dft_array_t Aux::fwd(IDFT::pointer dft, const Aux::dft_array_t& arr, int axis)
{
    Aux::dft_array_t ret = arr; 
    dft->fwd1b(ret.data(), ret.data(), ret.cols(), ret.rows(), !axis);
    return ret;

    // return doit1b(arr, axis,
    //               [&](const complex_t* in_data,
    //                   complex_t* out_data,
    //                   int nrows, int ncols) {
    //     dft->fwd1b(in_data, out_data, nrows, ncols);
    // });
}

Aux::dft_array_t Aux::inv(IDFT::pointer dft, const Aux::dft_array_t& arr, int axis)
{
    Aux::dft_array_t ret = arr; 
    dft->inv1b(ret.data(), ret.data(), ret.cols(), ret.rows(), !axis);
    return ret;
    // return doit1b(arr, axis,
    //               [&](const complex_t* in_data,
    //                   complex_t* out_data,
    //                   int nrows, int ncols) {
    //     dft->inv1b(in_data, out_data, nrows, ncols);
    // });
}
