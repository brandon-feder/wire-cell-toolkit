#include "WireCellUtil/RayTiling.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/ExecMon.h"

#include <random>
#include <iostream>

using namespace WireCell;
using namespace WireCell::RayGrid;

#include "raygrid.h"

const double pitch_magnitude = 5;
const double gaussian = 3;
const double width = 4000;
const double height = 4000;

std::vector<Point> random_points(int ndepos = 1000, int neles = 10)
{
    std::vector<Point> points;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> position(0, std::max(width, height));
    std::normal_distribution<double> spread(0.0, gaussian);
    for (int idepo = 0; idepo < ndepos; ++idepo) {
        Point cp(0, position(generator), position(generator));
        for (int iele = 0; iele < neles; ++iele) {
            Point delta(0, spread(generator), spread(generator));
            points.push_back(cp + delta);
        }
    }
    return points;
}

int main()
{
    ExecMon em("starting");

    auto raypairs = make_raypairs(width, height, pitch_magnitude);
    const int nlayers = raypairs.size();

    em("make ray pairs");

    std::vector<Point> points = random_points();

    Coordinates coords(raypairs);
    const auto& pitches = coords.pitch_dirs();
    const auto& centers = coords.centers();
    const auto& pitch_mags = coords.pitch_mags();

    em("make coordinates");

    std::vector<std::vector<Activity::value_t> > measures(nlayers);
    for (size_t ipt = 0; ipt < points.size(); ++ipt) {
        const auto& p = points[ipt];
        for (int ilayer = 0; ilayer < nlayers; ++ilayer) {
            const auto& pit = pitches[ilayer];
            const auto& cen = centers[ilayer];
            const auto rel = p - cen;
            const int pit_ind = pit.dot(rel) / pitch_mags[ilayer];
            auto& m = measures[ilayer];
            if (pit_ind < 0) {
                std::cerr << "Generated negative pitch index: " << pit_ind << " on L" << ilayer << " p=" << p
                          << std::endl;
                continue;
            }
            if ((int) m.size() <= pit_ind) {
                m.resize(pit_ind + 1, 0.0);
            }
            m[pit_ind] += 1.0;
        }
    }

    em("generated activity");

    activities_t activities;
    for (int ilayer = 0; ilayer < nlayers; ++ilayer) {
        auto& m = measures[ilayer];
        std::cerr << ilayer << ": " << m.size() << std::endl;
        Activity activity(ilayer, {m.begin(), m.end()});
        Assert(!activity.empty());
        activities.push_back(activity);
    }

    em("filled activity");

    blobs_t blobs;
    for (int i = 0; i < 100; ++i) {
        blobs = make_blobs(coords, activities);
    }
    em("made clusers 100 times");
    std::cerr << blobs.size() << " blobs\n";

    std::cerr << em.summary() << std::endl;
    return 0;
}
