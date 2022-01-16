#include "WireCellUtil/Pimpos.h"

#include "TCanvas.h"
#include "TArrow.h"
#include "TH1F.h"
#include "TLine.h"

#include <iostream>

#include "anode_loader.h"  // ignore everything in this file

using namespace WireCell;
using namespace std;

void draw_pimpos(TCanvas& canvas, const std::string& detector, std::vector<const Pimpos*>& pimposes)
{
    canvas.SetFixedAspectRatio(true);
    canvas.SetGridx();
    canvas.SetGridy();

    const double wire_extent = 5 * units::cm;  // half-length of wires
    const double fsize_mm = 1.5 * wire_extent / units::mm;

    const Point worigin = pimposes[2]->origin();

    TH1F* frame =
        canvas.DrawFrame(worigin[2] - fsize_mm, worigin[1] - fsize_mm, worigin[2] + fsize_mm, worigin[1] + fsize_mm);
    frame->SetTitle(Form("%s: Pitch (thick) and wire (thin) red=U, blue=V, xorigin=%.3fcm", detector.c_str(),
                         worigin[0] / units::cm));
    frame->SetXTitle("Transverse Z [mm]");
    frame->SetYTitle("Transverse Y [mm]");
    int colors[3] = {2, 4, 1};

    for (int iplane = 0; iplane < 3; ++iplane) {
        const Pimpos* pimpos = pimposes[iplane];

        const Vector wiredir = pimpos->axis(1);
        const Vector pitchdir = pimpos->axis(2);
        const Point origin = pimpos->origin();
        const Binning& binning = pimpos->region_binning();
        const double middle = binning.min() + 0.5 * binning.span();

        for (int ipitch = 0; ipitch <= binning.nbins(); ++ipitch) {
            const double pitch1 = binning.edge(ipitch);
            const double pitch2 = binning.edge(ipitch + 1);

            if (std::abs(middle - pitch1) > wire_extent) {
                continue;  // stupid way to limit the drawing
            }
            if (std::abs(middle - pitch2) > wire_extent) {
                continue;  // stupid way to limit the drawing
            }

            const Vector vpitch1 = origin + pitchdir * pitch1;
            const Vector vpitch2 = origin + pitchdir * pitch2;
            const Ray r_pitch(vpitch1, vpitch2);
            const Vector vwire = 1.2 * wiredir * wire_extent;
            const Ray r_wire(vpitch1 - vwire, vpitch1 + vwire);

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
}

int main(int argc, char* argv[])
{
    std::string detector = "uboone";
    if (argc > 1) {
        detector = argv[1];
    }
    auto anode_tns = anode_loader(detector);

    // output PDF
    std::string pdffile = argv[0];
    pdffile += "-" + detector + ".pdf";
    cerr << "Drawing to " << pdffile << endl;

    TCanvas canvas("c", "c", 500, 500);

    canvas.Print((pdffile + "[").c_str(), "pdf");

    for (const auto& anode_tn : anode_tns) {
        cerr << "Getting: " << anode_tn << "\n";
        auto iap = Factory::find_tn<IAnodePlane>(anode_tn);

        for (auto face : iap->faces()) {
            std::vector<const Pimpos*> pimposes;
            for (auto plane : face->planes()) {
                pimposes.push_back(plane->pimpos());
            }
            draw_pimpos(canvas, detector, pimposes);
            canvas.Print(pdffile.c_str(), "pdf");
        }
    }

    canvas.Print((pdffile + "]").c_str(), "pdf");
    return 0;
}
