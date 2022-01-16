#include "WireCellAux/DepoTools.h"

using namespace WireCell;


// perhaps this should be exported?
struct DepoGenChild {
    IDepo::pointer depo;
    size_t gen, child;
};
using DepoGenChilds = std::vector<DepoGenChild>;


static void push_depo(DepoGenChilds& dp,
                      IDepo::pointer depo,
                      size_t gen = 0, size_t childid = 0)
{
    dp.emplace_back(DepoGenChild{depo, gen, childid});
    auto prior = depo->prior();
    if (!prior) {
        return;
    }
    push_depo(dp, prior, gen + 1, dp.size());
}


static
DepoGenChilds flatten(const IDepo::vector& depos)
{
    DepoGenChilds ret;
    for (auto depo : depos) {
        push_depo(ret, depo);
    }
    return ret;
}


static
DepoGenChilds fake_flatten(const IDepo::vector& depos)
{
    DepoGenChilds ret;
    for (auto depo : depos) {
        ret.push_back(DepoGenChild{depo, 0,0});
    }
    return ret;
}


void Aux::fill(Array::array_xxf& data, Array::array_xxi& info, 
               const IDepo::vector& your_depos, bool prior)
{
    DepoGenChilds fdepos;
    if (prior) {
        fdepos = flatten(your_depos);
    }
    else {
        fdepos = fake_flatten(your_depos);
    }

    const size_t nfdepos = fdepos.size();

    size_t nrows = fdepos.size();

    // time, charge, x, y, z, dlong, dtran
    const size_t ndata = 7;
    data = Array::array_xxf::Zero(nrows, ndata);

    // ID, pdg, gen, child
    const size_t ninfo = 4;
    info = Array::array_xxi::Zero(nrows, ninfo);

    for (size_t idepo = 0; idepo != nfdepos; ++idepo) {
        const auto& depogc = fdepos[idepo];
        const auto& depo = depogc.depo;
        data(idepo, 0) = depo->time();
        data(idepo, 1) = depo->charge();
        data(idepo, 2) = depo->pos().x();
        data(idepo, 3) = depo->pos().y();
        data(idepo, 4) = depo->pos().z();
        data(idepo, 5) = depo->extent_long();
        data(idepo, 6) = depo->extent_tran();
        info(idepo, 0) = depo->id();
        info(idepo, 1) = depo->pdg();
        info(idepo, 2) = depogc.gen;
        info(idepo, 3) = depogc.child;
    }

}

