#include "WireCellGen/DepoPlaneX.h"
#include "WireCellIface/SimpleDepo.h"

#include <iostream>

using namespace WireCell;

void test_depoplanex()
{
    const double tunit = units::us;  // for display

    Gen::DepoPlaneX dpx(1 * units::cm);
    std::vector<double> times{10., 25., 50., 100.0, 500.0, 1000.0};  // us
    std::vector<double> xes{100.0, 0.0, 50.0, 10.0, 75.0};           // cm

    for (auto t : times) {
        t *= tunit;
        for (auto x : xes) {
            x *= units::cm;
            auto depo = std::make_shared<SimpleDepo>(t, Point(x, 0.0, 0.0));
            auto newdepo = dpx.add(depo);
            std::cerr << "\tx=" << x / units::cm << "cm, t=" << newdepo->time() / tunit << "us\n";
        }
        std::cerr << "^ t=" << t / tunit << "us, fot=" << dpx.freezeout_time() / tunit << "us\n";
    }
    dpx.freezeout();
    double fot = dpx.freezeout_time();
    std::cerr << dpx.frozen_queue().size() << " frozen with fot=" << fot / tunit << "us, " << dpx.working_queue().size()
              << " working\n";

    auto removed = dpx.pop(fot / 2.0);
    std::cerr << "Popped " << removed.size() << " at " << fot / 2.0 / tunit << std::endl;
    for (auto depo : removed) {
        std::cerr << "\tx=" << depo->pos().x() / units::cm << "cm, t=" << depo->time() / tunit << "us";
        std::cerr << " <--- ";
        std::cerr << "x=" << depo->prior()->pos().x() / units::cm << "cm, t=" << depo->prior()->time() / tunit
                  << "us\n";
    }
    std::cerr << "Frozen " << dpx.frozen_queue().size() << std::endl;
    for (auto depo : dpx.frozen_queue()) {
        std::cerr << "\tx=" << depo->pos().x() / units::cm << "cm, t=" << depo->time() / tunit << "us";
        std::cerr << " <--- ";
        std::cerr << "x=" << depo->prior()->pos().x() / units::cm << "cm, t=" << depo->prior()->time() / tunit
                  << "us\n";
    }
    std::cerr << "Left " << dpx.working_queue().size() << std::endl;
    for (auto depo : dpx.working_queue()) {
        std::cerr << "\tx=" << depo->pos().x() / units::cm << "cm, t=" << depo->time() / tunit << "us";
        std::cerr << " <--- ";
        std::cerr << "x=" << depo->prior()->pos().x() / units::cm << "cm, t=" << depo->prior()->time() / tunit
                  << "us\n";
    }
}

int main() { test_depoplanex(); }
