/*
  Test out the IRandom implementation Gen::Random.
 */

#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IRandom.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellUtil/ExecMon.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <complex>
#include <vector>

using namespace std;
using namespace WireCell;

double normalize(std::complex<double> val) { return std::abs(val); }
double normalize(double val) { return val; }
double normalize(int val) { return (double) val; }

template <typename NumType, int nbins = 10, int nstars = 100, int ntries = 100000>
void histify(std::function<NumType()> gen)
{
    int hist[nbins + 2] = {};
    for (int count = 0; count < ntries; ++count) {
        double num = normalize(gen());
        ++num;  // shift to accommodate under/overflow
        if (num <= 0) num = 0;
        if (num > nbins) num = nbins + 1;
        ++hist[(int) (0.5 + num)];
    }
    for (int bin = 0; bin < nbins + 2; ++bin) {
        int bar = (hist[bin] * nstars) / ntries;

        char sbin = '0' + (bin - 1);
        if (bin == 0) {
            sbin = '-';
        }
        if (bin == nbins + 1) {
            sbin = '+';
        }
        cout << sbin << ": " << std::string(bar, '*') << std::endl;
    }
}

void test_named(std::string generator_name)
{
    const std::string gen_random_name = "Random";
    auto rnd = Factory::lookup<IRandom>(gen_random_name);
    auto rndcfg = Factory::lookup<IConfigurable>(gen_random_name);

    {
        auto cfg = rndcfg->default_configuration();
        cfg["generator"] = generator_name;
        rndcfg->configure(cfg);
    }

    // Beware, this is evil.  Busting out the shared pointer and using
    // histify<> here is just to save some typing in this test.  It's
    // okay in this test because histify<> goes not live longer than
    // we hold the shared pointer.
    IRandom* ptr = &(*rnd);

    cout << "binomial(9,0.5)\n";
    histify<int>(std::bind(&IRandom::binomial, ptr, 9, 0.5));

    cout << "normal(5.0,3.0)\n";
    histify<double>(std::bind(&IRandom::normal, ptr, 5.0, 3.0));

    cout << "uniform(0.0,10.0)\n";
    histify<double>(std::bind(&IRandom::uniform, ptr, 0.0, 10.0));
    cout << "uniform(2.0,5.0)\n";
    histify<double>(std::bind(&IRandom::uniform, ptr, 2.0, 5.0));

    cout << "range(0,10)\n";
    histify<int>(std::bind(&IRandom::range, rnd, 0, 10));
    cout << "range(2,4)\n";
    histify<int>(std::bind(&IRandom::range, rnd, 2, 4));
}

void test_repeat()
{
    auto rnd = Factory::lookup<IRandom>("Random");

    const int ntries = 5;
    std::vector<double> v1(ntries), v2(ntries);
    for (int ind = 0; ind < ntries; ++ind) {
        v1[ind] = rnd->normal(0, 1);
    }
    for (int ind = 0; ind < ntries; ++ind) {
        v2[ind] = rnd->normal(0, 1);
    }
    for (size_t ind = 0; ind < v1.size(); ++ind) {
        cerr << v1[ind] << "\t" << v2[ind] << endl;
        Assert(std::abs(v1[ind] - v2[ind]) > 1.0e-8);
    }
}

int main()
{
    ExecMon em("starting");
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    em("plugged in");

    cout << "DEFAULT:\n";
    test_named("default");
    em("default generator");

    cout << "\nTWISTER:\n";
    test_named("twister");
    em("twister generator");

    cout << "\nBOGUS:\n";
    test_named("bogus");
    em("bogus generator");

    test_repeat();
    em("test repeat");

    cout << em.summary() << endl;

    return 0;
}
