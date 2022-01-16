/**
   This provides std::vector and Eigen::Array typed interface to an
   IDFT.
 */

#ifndef WIRECELL_AUX_DFTTOOLS
#define WIRECELL_AUX_DFTTOOLS

#include "WireCellIface/IDFT.h"
#include <vector>
#include <Eigen/Core>

namespace WireCell::Aux {

    using complex_t = IDFT::complex_t;

    // std::vector based functions

    using dft_vector_t = std::vector<complex_t>;

    // 1D with vectors

    inline dft_vector_t fwd(IDFT::pointer dft, const dft_vector_t& seq)
    {
        dft_vector_t ret(seq.size());
        dft->fwd1d(seq.data(), ret.data(), ret.size());
        return ret;
    }

    inline dft_vector_t inv(IDFT::pointer dft, const dft_vector_t& spec)
    {
        dft_vector_t ret(spec.size());
        dft->inv1d(spec.data(), ret.data(), ret.size());
        return ret;
    }

    // Eigen array based functions

    /// A complex, 2D array.  Use Array::cast<type>() if you need to
    /// convert to/from real.
    using dft_array_t = Eigen::ArrayXXcf;
    
    // 2D with Eigen arrays.  Use eg arr.cast<complex_>() to provde
    // from real or arr.real()() to convert result to real.

    // Transform both dimesions.
    dft_array_t fwd(IDFT::pointer dft, const dft_array_t& arr);
    dft_array_t inv(IDFT::pointer dft, const dft_array_t& arr);

    // Transform a 2D array along one axis.
    //
    // The axis identifies the logical array "dimension" over which
    // the transform is applied.  For example, axis=1 means the
    // transforms are applied along columns (ie, on a per-row basis).
    // Note: this is the same convention as held by numpy.fft.
    //
    // The axis is interpreted in the "logical" sense Eigen arrays
    // indexed as array(irow, icol).  Ie, the dimension traversing
    // rows is axis 0 and the dimension traversing columns is axis 1.
    // Note: internal storage order of an Eigen array may differ from
    // the logical order and indeed that of the array template type
    // order.  Neither is pertinent in setting the axis.
    dft_array_t fwd(IDFT::pointer dft, const dft_array_t& arr, int axis);
    dft_array_t inv(IDFT::pointer dft, const dft_array_t& arr, int axis);


}

#endif
