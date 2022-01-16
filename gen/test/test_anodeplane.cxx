#include "WireCellGen/AnodePlane.h"
#include "WireCellGen/AnodeFace.h"
#include "WireCellGen/WirePlane.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Exceptions.h"
#include <iostream>

#include "anode_loader.h"

using namespace WireCell;
using namespace std;

int main(int argc, char* argv[])
{
    std::string detector = "uboone";
    if (argc > 1) {
        detector = argv[1];
    }
    auto anode_tns = anode_loader(detector);

    for (std::string anode_tn : anode_tns) {
        auto iap = Factory::find_tn<IAnodePlane>(anode_tn);
        auto chans = iap->channels();
        cerr << "Anode " << anode_tn << " with " << chans.size() << " channels:\n";

        for (auto face : iap->faces()) {
            cerr << "face: " << face->ident() << "\n";
            std::vector<float> originx;
            for (auto plane : face->planes()) {
                cerr << "\tplane: " << plane->ident() << "\n";

                auto pimpos = plane->pimpos();
                cerr << "\torigin: " << pimpos->origin() / units::mm << "mm\n";
                for (int axis : {0, 1, 2}) {
                    cerr << "\taxis " << axis << ": " << pimpos->axis(axis) / units::mm << "mm\n";
                }
                originx.push_back(pimpos->origin()[0]);
            }

            float diff = std::abs(originx.front() - originx.back());
            if (diff > 0.1 * units::mm) {
                cerr << "ERROR, field response and wire location data do not match: diff = " << diff / units::mm
                     << "mm\n";
                cerr << "front: " << originx.front() / units::mm << "mm, back=" << originx.back() / units::mm
                     << "mm out of " << originx.size() << endl;
                THROW(ValueError() << errmsg{"field response and wire location data do not match"});
            }
        }
    }

    return 0;
}
