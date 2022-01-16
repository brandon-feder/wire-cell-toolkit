#ifndef WIRECELLTEST_UTILS
#define WIRECELLTEST_UTILS

#include "TStyle.h"
#include "TColor.h"

void set_palette(int pal = -1);
void set_palette(int pal)
{
    if (pal > 0) {
        gStyle->SetPalette(pal);
        return;
    }

    // custom
    const int ncolors = 50;
    static int colors[ncolors];
    static bool initialized = false;
    const int npars = 6;
    double stops[npars] = {0.00, 0.45, 0.49, 0.51, 0.55, 1.00};
    double reds[npars] = {0.00, 0.00, 1.00, 1.00, 1.00, 1.00};
    double greens[npars] = {0.00, 0.81, 1.00, 1.00, 0.20, 0.00};
    double blues[npars] = {1.00, 1.00, 1.00, 1.00, 0.00, 0.00};
    if (!initialized) {
        int FI = TColor::CreateGradientColorTable(npars, stops, reds, greens, blues, ncolors);
        for (int ind = 0; ind < ncolors; ++ind) {
            colors[ind] = FI + ind;
        }
        initialized = true;
        return;
    }
    gStyle->SetPalette(50, colors);
}

#endif
