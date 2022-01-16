
#include "WireCellUtil/NumpyHelper.h"
#include <iostream>
#include <string>

using FArray = Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic>;
using IArray = Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic>;

using WireCell::Numpy::load2d;

int main(int argc, char** argv)

{
    if (argc < 3) {
        std::cerr << "usage: check_numpy_depos depo-file.npz <SetN> [<FirstDepo> [<NDepos>]]\n";
    }
    const std::string fname = argv[1];
    const std::string nname = argv[2];
    size_t row_beg=0, nrows = 10;
    if (argc > 3) {
        row_beg = atoi(argv[3]);
    }
    if (argc > 4) {
        nrows = atoi(argv[4]);
    }

    FArray data;
    IArray info;
    load2d(data, "depo_data_" + nname, fname);
    load2d(info, "depo_info_" + nname, fname);
        
    assert (data.cols() == 7);
    assert (info.cols() == 4);
    
    const size_t ndepos = data.rows();
    assert(ndepos == (size_t)info.rows());

    const size_t row_end = std::min(row_beg + nrows, ndepos);

    for (size_t irow=row_beg; irow<row_end; ++irow) {
        
        std::cerr
            << "row=" << irow
            << " t=" << data(irow,0)
            << " id=" << info(irow,0)
            << " gen=" << info(irow,2)
            << " child=" << info(irow,3)
            << std::endl;
    }
    return 0;
}
