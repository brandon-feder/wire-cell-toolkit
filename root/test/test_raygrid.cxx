#include "WireCellUtil/RayGrid.h"
#include "WireCellUtil/Logging.h"
#include "WireCellUtil/Testing.h"

#include "TCanvas.h"
#include "TMarker.h"
#include "TText.h"
#include "TLine.h"
#include "TH1F.h"

#include <string>
#include <sstream>

using namespace WireCell;
using namespace WireCell::RayGrid;
using namespace std;

using spdlog::info;

void draw_ray(const Ray& ray, int color = 1)
{
    TLine l;
    l.SetLineColor(color);
    l.DrawLine(ray.first.z(), ray.first.y(), ray.second.z(), ray.second.y());
}

void draw_point(const Point& p, float size = 1, int style = 20, int color = 1);
void draw_point(const Point& p, float size, int style, int color)
{
    TMarker m;
    m.SetMarkerColor(color);
    m.SetMarkerSize(size);
    m.SetMarkerStyle(style);
    m.DrawMarker(p.z(), p.y());
}
void draw_text(const Point& p, std::string text)
{
    TText t;
    t.SetTextFont(82);
    t.SetTextSize(0.03);
    t.DrawText(p.z(), p.y(), text.c_str());
}
void draw_zero_crossing(const Coordinates& rg, layer_index_t il, layer_index_t im)
{
    auto p = rg.zero_crossing(il, im);
    draw_point(p, 1, 24);

    const auto& cs = rg.centers();
    const auto& c1 = cs[il];
    const auto& c2 = cs[im];
    draw_point(c1, 0.5, 20, 2);
    draw_point(c2, 0.5, 20, 4);

    const auto& rjs = rg.ray_jumps();
    const auto& j1 = rjs(il, im);
    const auto& j2 = rjs(im, il);

    draw_ray(Ray(c1, c1 + j1), 2);
    draw_ray(Ray(c2, c2 + j2), 4);

    std::stringstream ss;
    ss << "(" << il << "," << im << ")";
    draw_text(p, ss.str());
}

void draw_segments(const Coordinates& rg)
{
    const auto c0 = rg.centers()[0];
    const auto c1 = rg.centers()[1];
    const auto p0 = rg.pitch_dirs()[0];
    const auto p1 = rg.pitch_dirs()[1];
    const auto pm0 = rg.pitch_mags()[0];
    const auto pm1 = rg.pitch_mags()[1];

    const Vector ecks(1, 0, 0);

    for (int lind = 2; lind < rg.nlayers(); ++lind) {
        const auto& center = rg.centers()[lind];
        const auto& pdir = rg.pitch_dirs()[lind];
        const double pmag = rg.pitch_mags()[lind];
        const auto rdir = pdir.cross(ecks);

        Point next_center = center;

        int pind = -1;
        while (true) {
            ++pind;

            const auto pc = next_center;
            const double d0 = p0.dot(pc - c0);
            const double d1 = p1.dot(pc - c1);
            if (d0 < 0 or d0 > pm0) {
                break;
            }
            if (d1 < 0 or d1 > pm1) {
                break;
            }
            // handle anyt parallel layers special.

            Point pa, pb;
            if (1.0 - p0.dot(pdir) < 0.001) {  // layer 0 is parallel
                pa = rg.ray_crossing({1, 0}, {lind, pind});
                pb = rg.ray_crossing({1, 1}, {lind, pind});
            }
            else if (1.0 - p1.dot(pdir) < 0.001) {  // layer 1 is parallel
                pa = rg.ray_crossing({0, 0}, {lind, pind});
                pb = rg.ray_crossing({0, 1}, {lind, pind});
            }
            else {
                // normally, center is inside the "box" so sorting by
                // dot product of a vector from center to crossing
                // point and the ray direction means middle two are
                // closest.
                std::vector<Vector> crossings{
                    rg.ray_crossing({0, 0}, {lind, pind}), rg.ray_crossing({0, 1}, {lind, pind}),
                    rg.ray_crossing({1, 0}, {lind, pind}), rg.ray_crossing({1, 1}, {lind, pind})};

                sort(crossings.begin(), crossings.end(),
                     [&](const Vector& a, const Vector& b) { return rdir.dot(a - pc) < rdir.dot(b - pc); });
                pa = crossings[1];
                pb = crossings[2];
            }

            draw_ray(Ray(pa, pb));

            // recenter and move by one pitch
            next_center = 0.5 * (pa + pb) + pmag * pdir;  // this builds up errors
        }
    }
}

void draw_pairs(const ray_pair_vector_t& raypairs)
{
    for (const auto& rp : raypairs) {
        draw_ray(rp.first);
        draw_ray(rp.second);
    }
}

TH1F* draw_frame(TCanvas& canvas, std::string title, double xmin = -110, double ymin = -110, double xmax = +110,
                 double ymax = +110)
{
    auto* frame = canvas.DrawFrame(xmin, ymin, xmax, ymax);
    frame->SetTitle(title.c_str());
    return frame;
}

void draw(std::string fname, const Coordinates& rg, const ray_pair_vector_t& raypairs)
{
    TCanvas canvas("test_raygrid", "Ray Grid", 500, 500);
    auto draw_print = [&](std::string extra = "") { canvas.Print((fname + extra).c_str(), "pdf"); };

    draw_print("[");

    draw_frame(canvas, "rays", -10, -10);
    draw_segments(rg);
    draw_print();

    const int nbounds = raypairs.size();

    for (layer_index_t il = 0; il < nbounds; ++il) {
        for (layer_index_t im = 0; im < nbounds; ++im) {
            if (il < im) {
                draw_frame(canvas, Form("LAYER (%d,%d)", (int) il, (int) im));
                draw_pairs(raypairs);
                draw_zero_crossing(rg, il, im);

                for (grid_index_t ip = 0; ip < 100; ++ip) {
                    for (grid_index_t jp = 0; jp < 100; ++jp) {
                        coordinate_t one{il, ip}, two{im, jp};
                        auto p = rg.ray_crossing(one, two);
                        // cheat about knowing the bounds
                        if (p.z() < 0.0 or p.z() > 100.0) continue;
                        if (p.y() < 0.0 or p.y() > 100.0) continue;
                        draw_point(p, 1, 7);
                    }
                }

                draw_print();
            }
        }
    }

    draw_print("]");
}

void dump(std::string msg, const tensor_t& ar)
{
    info(msg);

    auto shape = ar.shape();
    std::stringstream ss;
    ss << "Dimensions: " << shape[0] << " " << shape[1] << " " << shape[2];
    info(ss.str());

    for (size_t i = 0; i < shape[0]; ++i) {
        for (size_t j = 0; j < shape[1]; ++j) {
            std::string line = "\t";
            for (size_t k = 0; k < shape[2]; ++k) {
                line += Form("%.1f", ar[i][j][k]);
            }
            info(line);
        }
        info("");
    }
}

void test_012(const Coordinates& rg)
{
    dump("a", rg.a());
    dump("b", rg.b());

    std::vector<double> ps;
    for (int a = 0; a < 2; ++a) {
        for (int b = 0; b < 2; ++b) {
            const double p = rg.pitch_location({0, a}, {1, b}, 2);
            info("a={} b={} p={}", a, b, p);
            ps.push_back(p);
        }
    }

    Assert(ps.front() != ps.back());
}

#include "raygrid.h"

int main(int argc, char* argv[])
{
    ray_pair_vector_t raypairs = make_raypairs();

    Coordinates rg(raypairs);

    test_012(rg);

    Assert(rg.nlayers() == (int) raypairs.size());

    for (int ind = 0; ind < rg.nlayers(); ++ind) {
        info("{} r1={} r2={} p={}[{}] c={}", ind, raypairs[ind].first, raypairs[ind].second, rg.pitch_dirs().at(ind),
             rg.pitch_mags().at(ind), rg.centers().at(ind));
    }

    std::string fname = argv[0];
    fname += ".pdf";
    draw(fname, rg, raypairs);
    return 0;
}
