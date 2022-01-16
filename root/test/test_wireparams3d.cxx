#include "WireCellGen/WireParams.h"
#include "WireCellUtil/Testing.h"

// 3d
#include "TView.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"

// 2d
#include "TH1F.h"
#include "TArrow.h"
#include "TLine.h"
#include "TAxis.h"

#include "MultiPdf.h"

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void draw_wires_3d(WireParams& wp)
{
    TView* view = TView::CreateView(1);
    view->SetRange(0, -10, -10, 5, 10, 10);
    view->ShowAxis();

    const Ray pitch_rays[3] = {wp.pitchU(), wp.pitchV(), wp.pitchW()};
    int colors[3] = {2, 4, 1};

    const Vector xaxis(1, 0, 0);

    for (int ind = 0; ind < 3; ++ind) {
        const Ray& pitch_ray = pitch_rays[ind];
        cout << ind << ": " << pitch_ray << endl;

        const Vector pitch_dir = ray_vector(pitch_ray);
        const Vector wire_dir = pitch_dir.cross(xaxis).norm();
        const Vector wire_point = pitch_ray.second + wire_dir;

        TPolyLine3D* pl = new TPolyLine3D(3);
        pl->SetPoint(0, pitch_ray.first.x(), pitch_ray.first.y(), pitch_ray.first.z());
        pl->SetPoint(1, pitch_ray.second.x(), pitch_ray.second.y(), pitch_ray.second.z());
        pl->SetPoint(2, wire_point.x(), wire_point.y(), wire_point.z());
        pl->SetLineColor(colors[ind]);
        pl->Draw();
    }
}

void draw_wires_2d(WireParams& wp)
{
    TH1F* frame = gPad->DrawFrame(0, -10, 20, 10);
    frame->SetTitle("Pitch (thick) and wire (thin) red=U, blue=V, +X (-drift) direction into page");
    frame->SetXTitle("Transverse Z direction");
    frame->SetYTitle("Transverse Y (W) direction");

    frame->GetYaxis()->SetAxisColor(3);
    frame->GetXaxis()->SetAxisColor(4);

    int colors[3] = {2, 4, 1};

    const Vector xaxis(1, 0, 0);
    const Ray pitch_rays[3] = {wp.pitchU(), wp.pitchV(), wp.pitchW()};

    for (int ind = 0; ind < 3; ++ind) {
        Ray r_pitch = pitch_rays[ind];
        r_pitch.first.x(0.0);
        r_pitch.second.x(0.0);

        const Vector d_pitch = ray_vector(r_pitch).norm();
        const Vector d_wire = d_pitch.cross(xaxis).norm();
        const Ray r_wire(r_pitch.second - d_wire, r_pitch.second + d_wire);
        // cout << ind
        //      << ": d_pitch=" << d_pitch
        //      << " d_wire=" << d_wire
        //      << " r_wire=" << r_wire
        //      << endl;

        TArrow* a_pitch =
            new TArrow(r_pitch.first.z(), r_pitch.first.y(), r_pitch.second.z(), r_pitch.second.y(), 0.01, "|>");
        a_pitch->SetLineColor(colors[ind]);
        a_pitch->SetLineWidth(2);

        TArrow* a_wire = new TArrow(r_wire.first.z(), r_wire.first.y(), r_wire.second.z(), r_wire.second.y(), 0.01);
        a_wire->SetLineColor(colors[ind]);

        a_pitch->Draw();
        a_wire->Draw();
    }
}

int main(int argc, char** argv)
{
    MultiPdf pdf(argv[0]);

    WireParams wp;

    const Ray& bbox = wp.bounds();

    cout << "Bounds: " << bbox << endl;

    cerr << "pU=" << wp.pitchU() << endl;
    cerr << "pV=" << wp.pitchV() << endl;
    cerr << "pW=" << wp.pitchW() << endl;

    pdf.canvas.Divide(2, 2);

    pdf.canvas.cd(1);
    draw_wires_3d(wp);
    pdf.canvas.cd(2);
    draw_wires_2d(wp);

    double pitch = 10.0;
    WireParams params;
    auto cfg = params.default_configuration();
    put(cfg, "pitch_mm.u", pitch);
    put(cfg, "pitch_mm.v", pitch);
    put(cfg, "pitch_mm.w", pitch);
    params.configure(cfg);

    pdf.canvas.cd(3);
    draw_wires_3d(wp);
    pdf.canvas.cd(4);
    draw_wires_2d(wp);

    pdf();

    return 0;
}
