/*
  Make some PDF file showing the concepts of Pimpos.
 */

#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/Testing.h"

#include "TCanvas.h"
#include "TArrow.h"
#include "TH1F.h"
#include "TLine.h"

#include <iostream>

using namespace std;
using namespace WireCell;

int main(int argc, char* argv[])
{
    // Match Garfield extent
    const int nwiresperplane = 21;

    // protoDUNE
    const double pitch = 5 * units::mm;
    const double angle = 35.707 * units::degree;
    const double wire_extent = pitch * 0.5 * (nwiresperplane - 1);

    std::vector<Vector> uvw_wire{Vector(0, cos(angle), sin(angle)),   // points Y>0, Z>0
                                 Vector(0, cos(angle), -sin(angle)),  // points Y>0, Z<0
                                 Vector(0, 1, 0)};

    // Pitch direction points generally in +Z direction (for "front" faces)
    std::vector<Vector> uvw_pitch{Vector(0, -sin(angle), cos(angle)), Vector(0, sin(angle), cos(angle)),
                                  Vector(0, 0, 1)};

    std::vector<Pimpos> pimpos;
    for (int iplane = 0; iplane < 3; ++iplane) {
        Pimpos p(nwiresperplane, -wire_extent, wire_extent, uvw_wire[iplane], uvw_pitch[iplane]);
        pimpos.push_back(p);

        const double wpdot = uvw_wire[iplane].dot(uvw_pitch[iplane]);
        Assert(std::abs(wpdot) < 1.0e-6);
    }

    TCanvas canvas("c", "c", 500, 500);

    canvas.SetFixedAspectRatio(true);
    canvas.SetGridx();
    canvas.SetGridy();

    const double fsize_mm = 0.8 * 2.0 * wire_extent / units::mm;
    TH1F* frame = canvas.DrawFrame(-fsize_mm, -fsize_mm, fsize_mm, fsize_mm);
    frame->SetTitle("Pitch (thick) and wire (thin) red=U, blue=V, +X (-drift) direction into page");
    frame->SetXTitle("Transverse Z [mm]");
    frame->SetYTitle("Transverse Y [mm]");
    int colors[3] = {2, 4, 1};

    for (int iplane = 0; iplane < 3; ++iplane) {
        const Vector wiredir = pimpos[iplane].axis(1);
        const Vector pitchdir = pimpos[iplane].axis(2);
        const Point origin = pimpos[iplane].origin();
        const Binning& binning = pimpos[iplane].region_binning();

        for (int ipitch = 0; ipitch <= binning.nbins(); ++ipitch) {
            const double pitch1 = binning.edge(ipitch);
            const double pitch2 = binning.edge(ipitch + 1);

            const Vector vpitch1 = origin + pitchdir * pitch1;
            const Vector vpitch2 = origin + pitchdir * pitch2;
            const Ray r_pitch(vpitch1, vpitch2);
            const Vector vwire = 1.2 * wiredir * wire_extent;
            const Ray r_wire(origin + vpitch1 - vwire, origin + vpitch1 + vwire);

            if (ipitch < binning.nbins()) {  // pitch is bin, wire is edge
                TArrow* a_pitch =
                    new TArrow(r_pitch.first.z() / units::mm, r_pitch.first.y() / units::mm,
                               r_pitch.second.z() / units::mm, r_pitch.second.y() / units::mm, 0.01, "|>");
                a_pitch->SetLineColor(colors[iplane]);
                a_pitch->SetLineWidth(2);
                a_pitch->Draw();
            }
            TArrow* a_wire = new TArrow(r_wire.first.z() / units::mm, r_wire.first.y() / units::mm,
                                        r_wire.second.z() / units::mm, r_wire.second.y() / units::mm, 0.01);
            a_wire->SetLineColor(colors[iplane]);

            a_wire->Draw();
        }
    }

    canvas.Print(Form("%s.pdf", argv[0]));

    return 0;
}
