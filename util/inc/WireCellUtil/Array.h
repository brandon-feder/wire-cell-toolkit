/**
   Wire Cell uses Eigen3 arrays for holding large block data like the
   waveforms from one plane of one readout frame.  This header
   provides a shim between Eigen3 and the rest of Wire Cell.

   There are a few important rules:

   - Eigen3 Arrays have element-wise arithmetic and Matrices have
     matrix-like arithmetic otherwise are similar.

   - They have .array() and .matrix() methods to produce one from the other.

   - Arrays are indexed by (row,col) order.

   - An Eigen3 Vector is a 1D Matrix of shape (N,1), again, (row,col).

   - A row, column or block from an array references the original
     array so can not live beyond it.

   - In Wire Cell large arrays are accessed via const shared pointer.

   Usage examples are given below.
 */

#ifndef WIRECELL_ARRAY
#define WIRECELL_ARRAY

#include "WireCellUtil/Waveform.h"

#include <Eigen/Core>

#include <memory>
#include <vector>

namespace WireCell {

    namespace Array {

        typedef Eigen::ArrayXf array_xf;
        typedef Eigen::ArrayXcf array_xc;

        /// A 16 bit short integer 2D array.
        typedef Eigen::Array<short, Eigen::Dynamic, Eigen::Dynamic> array_xxs;

        /// Integer
        typedef Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic> array_xxi;

        /// Integer
        typedef Eigen::Array<long, Eigen::Dynamic, Eigen::Dynamic> array_xxl;

        /// A real, 2D array
        typedef Eigen::ArrayXXf array_xxf;

        /// A complex, 2D array
        typedef Eigen::ArrayXXcf array_xxc;

        /** Perform full, 2D discrete Fourier transform on a real 2D
            array.

            The full 2D DFT first performs a 1D DFT (real->complex) on
            each individual row and then a 1D DFT (complex->complex)
            on each resulting column.

            const_shared_array_xxf arr = ...;
            const_shared_array_xxc spec = dft(*arr);

            // ...

            const_shared_array_xxf arr2 = idft(*spec);
         */
        array_xxc dft(const array_xxf& arr);
        array_xxf idft(const array_xxc& arr);

        /** Partial, 1D DFT and inverse DFT along one dimension of an
         * array.  Each row is transformed if dim=0, each column if
         * dim=1.  The transfer is either real->complex (rc),
         * complex->complex(cc) or complex->real(cr).
         *
         * The full 2D DFT should be used unless an intermediate
         * filter is required as it will avoid producing some
         * temporaries.
         *
         * Conceptually:
         *
         *    auto xxc = dft(xxf);
         *
         * is equivalent to
         *
         *    auto tmp = dft_rc(xxf, 0);
         *    auto xxc = dft_cc(tmp, 1);
         *
         * and:
         *
         *     auto xxf = idft(xxc)
         *
         * is equivalent to:
         *
         *     auto tmp = idft_cc(xxc, 1);
         *     auto xxf = idft_rc(tmp, 0);
         */
        array_xxc dft_rc(const array_xxf& arr, int dim = 0);
        array_xxc dft_cc(const array_xxc& arr, int dim = 1);
        array_xxc idft_cc(const array_xxc& arr, int dim = 1);
        array_xxf idft_cr(const array_xxc& arr, int dim = 0);

        /** Perform 2D deconvolution.

            This will perform a 2D forward DFT, do an
            element-by-element multiplication of that
            periodicity/frequency space matrix by the filter and then
            perform an 2D inverse DFT.

         */
        array_xxf deconv(const array_xxf& arr, const array_xxc& filter);

        /** downsample a 2D array along one axis by k
         *  simple average of all numbers in a bin
         *  e.g: MxN -> Mxfloor(N/k)
         *  extra rows/cols are ignored
         */
        array_xxf downsample(const array_xxf& in, const unsigned int k, const int dim = 0);

        /** upsample a 2D array along one axis by k
         *  e.g: MxN -> MxN*k
         *  all numbers in a new bin are assigned with same value
         */
        array_xxf upsample(const array_xxf& in, const unsigned int k, const int dim = 0);

        /** put a mask on in
         *  in and mask need to have same shape
         *  values > th in mask are considered pass
         */
        array_xxf mask(const array_xxf& in, const array_xxf& mask, const float th = 0.5);

        /** linear baseline subtraction along row direction
         */
        array_xxf baseline_subtraction(const array_xxf& in);

    }  // namespace Array
}  // namespace WireCell

#endif
