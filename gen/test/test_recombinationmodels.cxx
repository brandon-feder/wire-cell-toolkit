#include "WireCellGen/RecombinationModels.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Testing.h"

#include <vector>
#include <iostream>

/* From Brooke corresponding to example data used:
To convert "dE" to "n", the following was used:
fNumIonElectrons = fGeVToElectrons * 1.e-3 * e;
where
constexpr double kGeVToElectrons = 4.237e7; ///< 23.6eV per ion pair, 1e9 eV/GeV

    "depos": [
        {"x":107.42, "y":-23.175, "z":58.1284, "q":0.0797872, "t":3284.98, "s":0.0208263, "n":3380},
        {"x":107.419, "y":-23.15, "z":58.1216, "q":0.0533235, "t":3284.98, "s":0.0311329, "n":2259},
        {"x":107.418, "y":-23.1345, "z":58.1173, "q":0.000891013, "t":3284.98, "s":0.000947381, "n":37},
        {"x":107.417, "y":-23.1195, "z":58.1133, "q":0.0433082, "t":3284.98, "s":0.0301825, "n":1834},

*/
using namespace WireCell;
using namespace std;

const std::vector<double> evec{0.0797872, 0.0533235, 0.000891013, 0.02};
const std::vector<double> svec{0.0208263, 0.0311329, 0.000947381, 0.01};
const std::vector<int> nvec{3380, 2259, 37, 847};

void spin(IRecombinationModel& model, const std::string& label)
{
    cerr << "Model: " << label << endl;
    for (size_t ind = 0; ind < nvec.size(); ++ind) {
        // values from larsoft dump
        double lsenergy = evec[ind] * units::MeV;
        double lsstep = svec[ind] * units::cm;
        double lscharge = nvec[ind] * (-1.0 * units::eplus);
        double dEdX = lsenergy / lsstep;

        double gotcharge = model(lsenergy, lsstep);
        cerr << "\tdE=" << lsenergy / units::keV << " keV, "
             << "dX=" << lsstep / units::micrometer << " um, "
             << "dE/dX=" << dEdX / (units::MeV / units::cm) << " MeV/cm, "
             << "dQls=" << lscharge / (-1 * units::eplus) << " ele, "
             << "dQ=" << gotcharge / (-1 * units::eplus) << " ele\n";
    }
}
void test_larsoft()
{
    Gen::MipRecombination model(1.0);
    spin(model, "larsoft");
}
void test_mip()
{
    Gen::MipRecombination model;
    spin(model, "mip");
}
void test_birks()
{
    Gen::BirksRecombination model;
    spin(model, "birks");
}
void test_box()
{
    Gen::BoxRecombination model;
    spin(model, "box");
}

int main()
{
    test_larsoft();
    test_mip();
    test_birks();
    test_box();
    return 0;
}
