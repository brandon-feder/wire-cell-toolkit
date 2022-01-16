#include "WireCellIface/IDFT.h"

#include <vector>
#include <utility>              // std::swap since c++11

using namespace WireCell;

IDFT::~IDFT() {}

// Trivial default "batched" implementations.  If your concrete
// implementation provides some kind of "batch optimization", such as
// with FFTW3's advanced interface or with some GPU FFT library,
// override these dumb methods for the win.

void IDFT::fwd1b(const complex_t* in, complex_t* out,
                 int nrows, int ncols, int axis) const
{
    if (axis) { 
        for (int irow=0; irow<nrows; ++irow) {
            fwd1d(in+irow*ncols, out+irow*ncols, ncols);
        }
    }
    else {
        this->transpose(in, out, nrows, ncols);
        this->fwd1b(out, out, ncols, nrows, 1);
        this->transpose(out, out, ncols, nrows);
    }
}

void IDFT::inv1b(const complex_t* in, complex_t* out,
                 int nrows, int ncols, int axis) const
{
    if (axis) { 
        for (int irow=0; irow<nrows; ++irow) {
            inv1d(in+irow*ncols, out+irow*ncols, ncols);
        }
    }
    else {
        this->transpose(in, out, nrows, ncols);
        this->inv1b(out, out, ncols, nrows, 1);
        this->transpose(out, out, ncols, nrows);
    }
}

// Trivial default transpose.  Implementations, please override if you
// can offer something faster.

template<typename ValueType>
void transpose_type(const ValueType* in, ValueType* out,
                    int nrows, int ncols) 
{
    if (in != out) {
        for (int irow=0; irow<nrows; ++irow) {
            for (int icol=0; icol<ncols; ++icol) {
                out[icol*nrows + irow] = in[irow*ncols + icol];
            }
        }
        return;
    }
    
    // inplace adapated from https://stackoverflow.com/a/9320349 which
    // comes from
    // https://en.wikipedia.org/wiki/In-place_matrix_transposition#Non-square_matrices:_Following_the_cycles

    const int n = nrows;
    const int size = nrows*ncols;
    const int mn1 = (size - 1);
    std::vector<bool> visited(size);
    ValueType* first = out;
    const ValueType* last = first + size;
    ValueType* cycle = out;
    while (++cycle != last) {
        if (visited[cycle - first])
            continue;
        int a = cycle - first;
        do  {
            a = a == mn1 ? mn1 : (n * a) % mn1;
            std::swap(*(first + a), *cycle);
            visited[a] = true;
        } while ((first + a) != cycle);
    }

}


void IDFT::transpose(const IDFT::scalar_t* in, IDFT::scalar_t* out,
                     int nrows, int ncols) const
{
    transpose_type(in, out, nrows, ncols);
}
void IDFT::transpose(const IDFT::complex_t* in, IDFT::complex_t* out,
                     int nrows, int ncols) const
{
    transpose_type(in, out, nrows, ncols);
}
