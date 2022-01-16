#include "WireCellIface/IWireSelectors.h"
#include "WireCellGen/WireGenerator.h"
#include "WireCellGen/WireParams.h"
#include "WireCellUtil/Testing.h"

#include "TH1F.h"
#include "TArrow.h"
#include "TLine.h"

#include "MultiPdf.h"

#include <boost/range.hpp>

#include <iostream>
#include <cmath>

using namespace WireCell;
using namespace WireCell::Test;
using namespace std;

void test3D(MultiPdf& pdf, bool interactive)
{
    WireParams* params = new WireParams;
    double pitch = 10.0;
    auto cfg = params->default_configuration();
    put(cfg, "pitch_mm.u", pitch);
    put(cfg, "pitch_mm.v", pitch);
    put(cfg, "pitch_mm.w", pitch);
    params->configure(cfg);

    IWireParameters::pointer iwp(params);

    WireGenerator wg;
    WireGenerator::output_pointer wires;
    bool ok = wg(iwp, wires);
    Assert(ok);
    Assert(wires);
    AssertMsg(wires->size(), "Got no wires");

    const Ray& bbox = params->bounds();

    TH1F* frame = pdf.canvas.DrawFrame(bbox.first.z(), bbox.first.y(), bbox.second.z(), bbox.second.y());
    frame->SetTitle("red=U, blue=V, +X (-drift) direction into page");
    frame->SetXTitle("Transverse Z direction");
    frame->SetYTitle("Transverse Y (W) direction");

    int colors[3] = {2, 4, 1};

    vector<IWire::pointer> u_wires, v_wires, w_wires;
    copy_if(wires->begin(), wires->end(), back_inserter(u_wires), select_u_wires);
    copy_if(wires->begin(), wires->end(), back_inserter(v_wires), select_v_wires);
    copy_if(wires->begin(), wires->end(), back_inserter(w_wires), select_w_wires);
    size_t n_wires[3] = {u_wires.size(), v_wires.size(), w_wires.size()};

    double max_width = 5;
    for (auto wit = wires->begin(); wit != wires->end(); ++wit) {
        IWire::pointer wire = *wit;
        int iplane = wire->planeid().index();
        int index = wire->index();

        AssertMsg(n_wires[iplane], "Empty plane");
        double width = 1.0 + (((index + 1) * max_width) / n_wires[iplane]);

        const Ray ray = wire->ray();

        TArrow* a_wire = new TArrow(ray.first.z(), ray.first.y(), ray.second.z(), ray.second.y(), 0.01, "|>");
        a_wire->SetLineColor(colors[iplane]);
        a_wire->SetLineWidth(width);
        a_wire->Draw();
    }
    pdf();
}

int main(int argc, char** argv)
{
    MultiPdf pdf(argv[0]);
    test3D(pdf, argc > 1);
    return 0;
}
