#include "WireCellUtil/Array.h"
#include "WireCellUtil/Exceptions.h"

#include <unsupported/Eigen/FFT>

#include <algorithm>
#include <complex>

using namespace WireCell;
using namespace WireCell::Array;

// Need to use different planner for different input/output
// https://eigen.tuxfamily.org/dox/unsupported/ei__fftw__impl_8h_source.html

thread_local static Eigen::FFT<float> gEigenFFT_dft_1d;      // c2c fwd and inv
thread_local static Eigen::FFT<float> gEigenFFT_dft_r2c_1d;  // r2c fwd
thread_local static Eigen::FFT<float> gEigenFFT_dft_c2r_1d;  // c2r inv

// http://stackoverflow.com/a/33636445

WireCell::Array::array_xxc WireCell::Array::dft(const WireCell::Array::array_xxf& arr)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::MatrixXcf matc(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXcf fspec(ncols);  // frequency spectrum
        // gEigenFFT wants vectors, also input arr is const
        Eigen::VectorXf tmp = arr.row(irow);
        gEigenFFT_dft_r2c_1d.fwd(fspec, tmp);  // r2c
        matc.row(irow) = fspec;
    }

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows);                // periodicity spectrum
        gEigenFFT_dft_1d.fwd(pspec, matc.col(icol));  // c2c
        matc.col(icol) = pspec;
    }

    return matc;
}

WireCell::Array::array_xxc WireCell::Array::dft_rc(const WireCell::Array::array_xxf& arr, int dim)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::MatrixXcf matc(nrows, ncols);

    if (dim == 0) {
        for (int irow = 0; irow < nrows; ++irow) {
            Eigen::VectorXcf fspec(ncols);
            Eigen::VectorXf tmp = arr.row(irow);
            gEigenFFT_dft_r2c_1d.fwd(fspec, tmp);  // r2c
            matc.row(irow) = fspec;
        }
    }
    else if (dim == 1) {
        for (int icol = 0; icol < ncols; ++icol) {
            Eigen::VectorXcf fspec(nrows);
            Eigen::VectorXf tmp = arr.col(icol);
            gEigenFFT_dft_r2c_1d.fwd(fspec, tmp);  // r2c
            matc.col(icol) = fspec;
        }
    }
    return matc;
}

WireCell::Array::array_xxc WireCell::Array::dft_cc(const WireCell::Array::array_xxc& arr, int dim)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::MatrixXcf matc(nrows, ncols);

    matc = arr.matrix();

    if (dim == 0) {
        for (int irow = 0; irow < nrows; ++irow) {
            Eigen::VectorXcf pspec(ncols);
            gEigenFFT_dft_1d.fwd(pspec, matc.row(irow));  // c2c
            matc.row(irow) = pspec;
        }
    }
    else {
        for (int icol = 0; icol < ncols; ++icol) {
            Eigen::VectorXcf pspec(nrows);
            gEigenFFT_dft_1d.fwd(pspec, matc.col(icol));  // c2c
            matc.col(icol) = pspec;
        }
    }
    return matc;
}

WireCell::Array::array_xxf WireCell::Array::idft(const WireCell::Array::array_xxc& arr)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    // gEigenFFT works on matrices, not arrays, also don't step on const input
    Eigen::MatrixXcf partial(nrows, ncols);
    partial = arr.matrix();

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows);                   // wire spectrum
        gEigenFFT_dft_1d.inv(pspec, partial.col(icol));  // c2c
        partial.col(icol) = pspec;
    }

    // shared_array_xxf ret = std::make_shared<array_xxf> (nrows, ncols);
    array_xxf ret(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXf wave(ncols);                        // back to real-valued time series
        gEigenFFT_dft_c2r_1d.inv(wave, partial.row(irow));  // c2r
        ret.row(irow) = wave;
    }

    return ret;
}

WireCell::Array::array_xxc WireCell::Array::idft_cc(const WireCell::Array::array_xxc& arr, int dim)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    // gEigenFFT works on matrices, not arrays, also don't step on const input
    Eigen::MatrixXcf ret(nrows, ncols);
    ret = arr.matrix();

    if (dim == 1) {
        for (int icol = 0; icol < ncols; ++icol) {
            Eigen::VectorXcf pspec(nrows);
            gEigenFFT_dft_1d.inv(pspec, ret.col(icol));  // c2c
            ret.col(icol) = pspec;
        }
    }
    else if (dim == 0) {
        for (int irow = 0; irow < nrows; ++irow) {
            Eigen::VectorXcf pspec(ncols);
            gEigenFFT_dft_1d.inv(pspec, ret.row(irow));  // c2c
            ret.row(irow) = pspec;
        }
    }
    return ret;
}

WireCell::Array::array_xxf WireCell::Array::idft_cr(const WireCell::Array::array_xxc& arr, int dim)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    // gEigenFFT works on matrices, not arrays, also don't step on const input
    Eigen::MatrixXcf partial(nrows, ncols);
    partial = arr.matrix();

    array_xxf ret(nrows, ncols);

    if (dim == 0) {
        for (int irow = 0; irow < nrows; ++irow) {
            Eigen::VectorXf wave(ncols);                        // back to real-valued time series
            gEigenFFT_dft_c2r_1d.inv(wave, partial.row(irow));  // c2r
            ret.row(irow) = wave;
        }
    }
    else if (dim == 1) {
        for (int icol = 0; icol < ncols; ++icol) {
            Eigen::VectorXf wave(nrows);
            gEigenFFT_dft_c2r_1d.inv(wave, partial.col(icol));  // c2r
            ret.col(icol) = wave;
        }
    }
    return ret;
}

// this is a cut-and-paste mashup of dft() and idft() in order to avoid temporaries.
WireCell::Array::array_xxf WireCell::Array::deconv(const WireCell::Array::array_xxf& arr,
                                                   const WireCell::Array::array_xxc& filter)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();

    Eigen::MatrixXcf matc(nrows, ncols);
    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXcf fspec(ncols);  // frequency spectrum
        // gEigenFFT wants vectors, also input arr is const
        Eigen::VectorXf tmp = arr.row(irow);
        gEigenFFT_dft_r2c_1d.fwd(fspec, tmp);  // r2c
        matc.row(irow) = fspec;
    }

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows);                // periodicity spectrum
        gEigenFFT_dft_1d.fwd(pspec, matc.col(icol));  // c2c
        matc.col(icol) = pspec;
    }

    // deconvolution via multiplication in frequency space
    Eigen::MatrixXcf filt = matc.array() * filter;

    for (int icol = 0; icol < ncols; ++icol) {
        Eigen::VectorXcf pspec(nrows);                // wire spectrum
        gEigenFFT_dft_1d.inv(pspec, filt.col(icol));  // c2c
        filt.col(icol) = pspec;
    }

    array_xxf ret(nrows, ncols);

    for (int irow = 0; irow < nrows; ++irow) {
        Eigen::VectorXf wave(ncols);                     // back to real-valued time series
        gEigenFFT_dft_c2r_1d.inv(wave, filt.row(irow));  // c2r
        ret.row(irow) = wave;
    }

    return ret;
}

WireCell::Array::array_xxf WireCell::Array::downsample(const Array::array_xxf& in, const unsigned int k, const int dim)
{
    if (dim == 0) {
        Array::array_xxf out = Array::array_xxf::Zero(in.rows() / k, in.cols());
        for (unsigned int i = 0; i < in.rows(); ++i) {
            out.row(i / k) = out.row(i / k) + in.row(i);
        }
        return out / k;
    }
    if (dim == 1) {
        Array::array_xxf out = Array::array_xxf::Zero(in.rows(), in.cols() / k);
        for (unsigned int i = 0; i < in.cols(); ++i) {
            out.col(i / k) = out.col(i / k) + in.col(i);
        }
        return out / k;
    }
    THROW(ValueError() << errmsg{"dim should be 0 or 1"});
}

WireCell::Array::array_xxf WireCell::Array::upsample(const Array::array_xxf& in, const unsigned int k, const int dim)
{
    if (dim == 0) {
        Array::array_xxf out = Array::array_xxf::Zero(in.rows() * k, in.cols());
        for (unsigned int i = 0; i < in.rows() * k; ++i) {
            out.row(i) = in.row(i / k);
        }
        return out;
    }
    if (dim == 1) {
        Array::array_xxf out = Array::array_xxf::Zero(in.rows(), in.cols() * k);
        for (unsigned int i = 0; i < in.cols() * k; ++i) {
            out.col(i) = in.col(i / k);
        }
        return out;
    }
    THROW(ValueError() << errmsg{"dim should be 0 or 1"});
}

WireCell::Array::array_xxf WireCell::Array::mask(const Array::array_xxf& in, const Array::array_xxf& mask,
                                                 const float th)
{
    Array::array_xxf ret = Eigen::ArrayXXf::Zero(in.rows(), in.cols());
    if (in.rows() != mask.rows() || in.cols() != mask.cols()) {
        THROW(ValueError() << errmsg{"in.rows()!=mask.rows() || in.cols()!=mask.cols()"});
    }
    return (mask > th).select(in, ret);
}

WireCell::Array::array_xxf WireCell::Array::baseline_subtraction(const Array::array_xxf& in)
{
    Array::array_xxf ret = Eigen::ArrayXXf::Zero(in.rows(), in.cols());
    for (int ich = 0; ich < in.cols(); ++ich) {
        int sta = 0;
        int end = 0;
        for (int it = 0; it < in.rows(); ++it) {
            if (in(it, ich) == 0) {
                if (sta < end) {
                    for (int i = sta; i < end + 1; ++i) {
                        ret(i, ich) =
                            in(i, ich) - (in(sta, ich) + (i - sta) * (in(end, ich) - in(sta, ich)) / (end - sta));
                    }
                }
                sta = it + 1;  // first tick in ROI
            }
            else {
                end = it;  // last tick in ROI
            }
        }
    }
    return ret;
}