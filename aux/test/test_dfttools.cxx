#include "aux_test_dft_helpers.h"

#include "WireCellAux/DftTools.h"
#include "WireCellAux/FftwDFT.h"
#include "WireCellUtil/Waveform.h"

#include <iostream>
#include <memory>

using namespace WireCell;
using namespace WireCell::Aux::Test;

using real_t = float;
using RV = std::vector<real_t>;
using complex_t = std::complex<real_t>;
using CV = std::vector<complex_t>;

void test_1d_impulse(IDFT::pointer dft, int size = 64)
{
    RV rimp(size, 0);
    rimp[0] = 1.0;

    auto cimp = Aux::fwd(dft, Waveform::complex(rimp));
    assert_flat_value(cimp.data(), cimp.size());

    RV rimp2 = Waveform::real(Aux::inv(dft, cimp));
    assert_impulse_at_index(rimp2.data(), rimp2.size());
}

using FA = Eigen::Array<real_t, Eigen::Dynamic, Eigen::Dynamic>;
using CA = Eigen::Array<complex_t, Eigen::Dynamic, Eigen::Dynamic>;
using FARM = Eigen::Array<real_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using CARM = Eigen::Array<complex_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

void test_2d_impulse(IDFT::pointer dft, int nrows=16, int ncols=8)
{
    const size_t size = nrows*ncols;
    FA r = FA::Zero(nrows, ncols);
    r(0,0) = 1.0;
    dump("r", r);
    assert_impulse_at_index(r.data(), size);

    CA rc = r.cast<complex_t>();
    dump("rc", rc);
    assert_impulse_at_index(rc.data(), size);

    CA c = Aux::fwd(dft, rc);
    dump("c", c);
    assert_flat_value(c.data(), size);

    FA r2 = Aux::inv(dft, c).real();
    dump("r2", r2);
    assert_impulse_at_index(r2.data(), size);
}

void test_2d_eigen_transpose(IDFT::pointer dft)
{
    const int nrows=16;
    const int ncols=8;

    // where the impulse lives (off axis)
    const int imp_row = 1;
    const int imp_col = 10;

    FA r = FA::Zero(nrows, ncols); // shape:(16,8)
    dump("r", r);

    // do not remove the auto in this next line
    auto rt = r.transpose();    // shape:(8,16)
    dump("rt", rt);
    rt(imp_row, imp_col) = 1.0;

    auto c = Aux::fwd(dft, rt.cast<complex_t>());
    dump("c", c);

    auto r2 = Aux::inv(dft, c).real();
    dump("r2",r2);

    // transpose access
    const int nrowst = r2.rows();
    const int ncolst = r2.cols();

    for (int irow=0; irow<nrowst; ++irow) {
        for (int icol=0; icol<ncolst; ++icol) {
            float val = rt(irow, icol);
            float val2 = r2(irow, icol); // access with transposed indices
            std::cerr << "(" << irow << ","<< icol << "):" << val << " ? " << val2 << "\n";
            if (irow==imp_row and icol==imp_col) {
                assert(std::abs(val-1.0) < 1e-6);
                continue;
            }
            assert(std::abs(val) < 1e-6);    
        }
        std::cerr << "\n";
    }
}

void test_1b(IDFT::pointer dft, int axis, int nrows=8, int ncols=4)
{
    FA r = FA::Zero(nrows, ncols);
    r(0,0) = 1.0;
    dump("impulse", r);
    CA c = Aux::fwd(dft, r.cast<complex_t>(), axis);

    dump("spectra", c);
    std::cerr << c << std::endl;

    if (axis) {                 // transform along rows
        CA ct = c.transpose();      // convert to along columns (native Eigen storage order)
        c = ct;
        std::swap(nrows, ncols);
        dump("transpose", c);
        std::cerr << c << std::endl;
    }

    // first column has flat abs value of 1.0.
    assert_flat_value(c.data(), nrows, complex_t(1,0)); 
    // rest should be flat, zero value
    assert_flat_value(c.data()+nrows, nrows*ncols - nrows, complex_t(0,0)); 

}

int main(int argc, char* argv[])
{
    DftArgs args;
    int rc = make_dft_args(args, argc, argv);
    if (rc) { return rc; }
    auto idft = make_dft(args.tn, args.pi, args.cfg);

    test_1d_impulse(idft);
    test_2d_impulse(idft);
    test_2d_eigen_transpose(idft);
    test_1b(idft, 0);
    test_1b(idft, 1);

    return 0;
}
