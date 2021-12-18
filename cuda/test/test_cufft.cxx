#include <iostream>
#include <stdio.h>
#include "debug-helper.cpp"

#include "WireCellCuda/cuFftDFT.h"
#include "debug-helper.cpp"

using namespace WireCell;

using complex_t = IDFT::complex_t;

int main()
{
    auto testIn = (complex_t *) calloc(4, sizeof(complex_t));
    auto testOut = (complex_t *) malloc(4*sizeof(complex_t));

    testIn[0] = 1;

    printArr(testIn, 4);

    Cuda::cuFftDFT fftObj = Cuda::cuFftDFT();
    fftObj.fwd1d(testIn, testOut, 4);
    printArr(testOut, 4);
    return 0;
}
