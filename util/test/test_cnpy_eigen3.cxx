/**
   Eigen defaults column-major ordering (FORTRAN).

   Cnpy defaults to row-major ordering (C).

   This is like test_cnpy_eigen.cxx but uses the numpy helpers.

 */

#include "WireCellUtil/NumpyHelper.h"
#include "WireCellUtil/Array.h"

using ntype = int;

using ArrayType = Eigen::Array<ntype, Eigen::Dynamic, Eigen::Dynamic>;

const int Nrows = 3;
const int Ncols = 4;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr
            << "This test needs a numpy file\n"
            << "To generate it, run util/test/test_cnpy_eigen3.py\n";
        return 0;
    }

    std::string fname = argv[1];

    ArrayType narr;
    WireCell::Numpy::load2d(narr, "n_order", fname);
    assert(narr.rows() == Nrows);
    assert(narr.cols() == Ncols);

    ArrayType carr;
    WireCell::Numpy::load2d(carr, "c_order", fname);
    assert(carr.rows() == Nrows);
    assert(carr.cols() == Ncols);

    ArrayType farr;
    WireCell::Numpy::load2d(farr, "f_order", fname);
    assert(farr.rows() == Nrows);
    assert(farr.cols() == Ncols);

    std::cerr << "Expected:\n";
    for (int irow = 0; irow < Nrows; ++irow) {
        for (int icol=0; icol < Ncols; ++icol) {
            ntype val = icol + irow*Ncols;
            std::cerr << " " << val;
        }
        std::cerr << std::endl;
    }
    std::cerr << "N order:\n";
    for (int irow = 0; irow < Nrows; ++irow) {
        for (int icol=0; icol < Ncols; ++icol) {
            std::cerr << " " << narr(irow, icol);
        }
        std::cerr << std::endl;
    }
    std::cerr << "C order:\n";
    for (int irow = 0; irow < Nrows; ++irow) {
        for (int icol=0; icol < Ncols; ++icol) {
            std::cerr << " " << carr(irow, icol);
        }
        std::cerr << std::endl;
    }
    std::cerr << "F order:\n";
    for (int irow = 0; irow < Nrows; ++irow) {
        for (int icol=0; icol < Ncols; ++icol) {
            std::cerr << " " << farr(irow, icol);
        }
        std::cerr << std::endl;
    }

    for (int irow = 0; irow < Nrows; ++irow) {
        for (int icol=0; icol < Ncols; ++icol) {
            ntype val = icol + irow*Ncols;
            std::cerr << "["<<irow<<","<<icol<<"] = "
                      << val << " | " << carr(irow,icol) << " | " << farr(irow,icol) 
                      << std::endl;
            assert(narr(irow,icol) == val);
            assert(carr(irow,icol) == val);
            assert(farr(irow,icol) == val);
        }
    }

    return 0;
}
