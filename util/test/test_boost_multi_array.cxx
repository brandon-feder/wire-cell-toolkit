#include "WireCellUtil/Testing.h"

#include <boost/multi_array.hpp>

#include <iostream>

using namespace std;

int main()
{
    typedef boost::multi_array<double, 2> array_type;
    typedef array_type::index index;

    size_t l_size = 10;
    size_t t_size = 3;
    array_type ar(boost::extents[l_size][t_size]);
    cerr << "Dimensions: " << ar.dimensionality << endl;
    cerr << "Dimension 0 is size " << ar.shape()[0] << endl;
    cerr << "Dimension 1 is size " << ar.shape()[1] << endl;

    Assert(l_size == ar.shape()[0]);
    Assert(t_size == ar.shape()[1]);
    for (size_t l_ind = 0; l_ind < l_size; ++l_ind) {
        for (size_t t_ind = 0; t_ind < t_size; ++t_ind) {
            ar[l_ind][t_ind] = l_ind * t_ind;
        }
    }
    for (size_t t_ind = 0; t_ind < t_size; ++t_ind) {
        for (size_t l_ind = 0; l_ind < l_size; ++l_ind) {
            cerr << "\t[" << l_ind << "][" << t_ind << "] = " << ar[l_ind][t_ind];
        }
        cerr << endl;
    }

    array_type ar2(boost::extents[l_size][t_size]);
    ar2 = ar;
    Assert(l_size == ar2.shape()[0]);
    Assert(t_size == ar2.shape()[1]);

    array_type ar3 = ar;
    Assert(l_size == ar3.shape()[0]);
    Assert(t_size == ar3.shape()[1]);

    int nx = 3, ny = 4, nz = 5;

    typedef boost::multi_array<double, 3> array_type3;

    array_type3 B(boost::extents[nx][ny][nz]);
    typedef array_type3::index index3;
    for (index3 i = 0; i < nx; ++i) {
        for (index3 j = 0; j < ny; ++j) {
            for (index3 k = 0; k < nz; ++k) {
                const double foo = i * j * k;

                B[i][j][k] = foo;
            }
        }
    }

    return 0;
}
