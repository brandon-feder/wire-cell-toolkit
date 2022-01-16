// Test what happens with different choices of how we bin.

#include "MultiPdf.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Response.h"
#include "WireCellGen/RCResponse.h"

#include "TGraph.h"
#include "TH1F.h"

using namespace WireCell;
using namespace WireCell::Waveform;

struct FakeFR : public Response::Generator {
    double scale, shift;
    FakeFR(double scale, double shift=0) : scale(scale), shift(shift) {}

    virtual ~FakeFR() {}

    virtual double operator()(double time) const {
        double t2 = (time+shift)/scale;
        t2 *= t2;
        return t2*sin(t2*0.5*3.1415);
    }
};

// A fake field response
realseq_t fr(const Binning& bins)
{
    realseq_t ret(bins.nbins(), 0);

    const double xmin = bins.min();

    for (int ind=0; ind<bins.nbins(); ++ind) {
        const double x = bins.center(ind) - xmin;
        const double x2 = x*x;
        ret[ind] = x2 * sin(x2);
    }

    return ret;
}

realseq_t rebin(const realseq_t& arr, int rf)
{
    const double norm = 1.0/rf;
    realseq_t ret(arr.size()/rf, 0);
    for (size_t ind=0; ind<arr.size(); ++ind) {
        ret[ind/rf] += norm*arr[ind];
    }
    return ret;
}

struct Plotter {
    Test::MultiPdf& pdf;
    std::vector<TH1*> garbage;
    Plotter(Test::MultiPdf& pdf) : pdf(pdf) { }

    void draw(const realseq_t& arr, const Binning& bins,
              std::string name="", std::string title="") {
        auto h = new TH1F(name.c_str(),
                          title.c_str(),
                          bins.nbins(), bins.min(), bins.max());
        for (size_t ind=0; ind<arr.size(); ++ind) {
            //const double x = bins.center(ind);
            h->SetBinContent(ind+1, arr[ind]);
        }
        h->Draw();
        garbage.push_back(h);
        pdf();
    }
};


int main(int argc, char* argv[])
{
    Test::MultiPdf mpdf(argv[0]);
    Plotter p(mpdf);

    const double tmin=0;
    const double tmax_long=100*units::us;
    const double tmax_short=10*units::us;
    const int rebinfactor=5;
    const int ncoarse=200;
    const int nfine=rebinfactor*ncoarse;

    Response::ColdElec ce;
    FakeFR fr(tmax_long/3);
    FakeFR frs(tmax_long/3, 1.0*units::us);

    Binning cbin_long(ncoarse, tmin, tmax_long);
    Binning fbin_long(nfine, tmin, tmax_long);
    Binning cbin_short(ncoarse, tmin, tmax_short);
    Binning fbin_short(nfine, tmin, tmax_short);

    auto cfr = fr.generate(cbin_long);
    auto ffr = fr.generate(fbin_long);
    auto cfrs = frs.generate(cbin_long);
    auto ffrs = frs.generate(fbin_long);

    p.draw(cfr, cbin_long, "cfr", "Coarse FR");
    p.draw(cfrs, cbin_long, "cfrs", "Coarse FR Shifted");
    p.draw(ffr, fbin_long, "ffr", "Fine FR");
    p.draw(ffrs, fbin_long, "ffrs", "Fine FR Shifted");
    
    auto cce = ce.generate(cbin_short);
    auto fce = ce.generate(fbin_short);
    
    p.draw(cce, cbin_short, "cce", "Coarse CE");
    p.draw(fce, fbin_short, "fce", "Fine CE");

    // convolve + rebin fine->coarse
    auto fcc = linear_convolve(ffr, fce);
    p.draw(fcc, fbin_long, "fcc", "Fine conv");
    auto ccc2 = rebin(fcc, rebinfactor);
    p.draw(ccc2, cbin_long, "ccc2", "Coarse rebin conv");
    auto fccs = linear_convolve(ffrs, fce);
    p.draw(fccs, fbin_long, "fccs", "Fine conv shifted");
    auto cccs2 = rebin(fccs, rebinfactor);
    p.draw(cccs2, cbin_long, "cccs2", "Coarse rebin conv shifted");

    // rebin fine->coarse + convolve
    auto ccc = linear_convolve(cfr, cce);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        ccc[ind] *= rebinfactor;
    }
    p.draw(ccc, cbin_long, "ccc", "Coarse native conv");
    auto cccs = linear_convolve(cfrs, cce);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        cccs[ind] *= rebinfactor;
    }
    p.draw(cccs, cbin_long, "cccs", "Coarse native conv shifted");

    realseq_t ccc3(ccc.size(), 0);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        ccc3[ind] = fcc[ind*rebinfactor] - ccc[ind];
    }
    p.draw(ccc3, cbin_long, "ccc3", "Coarse sampled differences");
    realseq_t cccs3(ccc.size(), 0);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        cccs3[ind] = fccs[ind*rebinfactor] - cccs[ind];
    }
    p.draw(cccs3, cbin_long, "cccs3", "Coarse sampled differences, shifted");

    realseq_t ccc4(ccc.size(), 0);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        ccc4[ind] = ccc2[ind] - ccc[ind];
    }
    p.draw(ccc4, cbin_long, "ccc4", "Coarse differences");

    realseq_t ccc5(ccc.size(), 0);
    for (size_t ind=0; ind<ccc.size(); ++ind) {
        ccc5[ind] = ccc2[ind] - fcc[ind*rebinfactor];
    }
    p.draw(ccc5, cbin_long, "ccc5", "Coarse native/sampled differences");


    return 0;
}
