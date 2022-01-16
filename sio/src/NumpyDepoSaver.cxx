#include "WireCellSio/NumpyDepoSaver.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Array.h"
#include "WireCellUtil/NumpyHelper.h"
#include "WireCellAux/DepoTools.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <tuple>

WIRECELL_FACTORY(NumpyDepoSaver, WireCell::Sio::NumpyDepoSaver, WireCell::IDepoFilter, WireCell::IConfigurable)

using namespace WireCell;
using WireCell::Numpy::save2d;

Sio::NumpyDepoSaver::NumpyDepoSaver()
    : Aux::Logger("NumpyDepoSaver", "io")
{
}

Sio::NumpyDepoSaver::~NumpyDepoSaver() {}

WireCell::Configuration Sio::NumpyDepoSaver::default_configuration() const
{
    Configuration cfg;

    // The output file name to write.  Only compressed (zipped) Numpy
    // files are supported.  Writing is always in "append" mode.  It's
    // up to the user to delete a previous instance of the file if
    // it's old contents are not wanted.
    cfg["filename"] = m_filename;
    return cfg;
}

void Sio::NumpyDepoSaver::configure(const WireCell::Configuration& config)
{
    m_filename = get<std::string>(config, "filename", m_filename);
}

#if 0                           // moved to DepoTools
typedef std::tuple<IDepo::pointer, size_t, size_t> depo_gen_child;
typedef std::vector<depo_gen_child> depos_with_prior;

static void push_depo(depos_with_prior& dp, WireCell::IDepo::pointer depo, size_t gen = 0, size_t childid = 0)
{
    dp.push_back(depo_gen_child(depo, gen, childid));
    auto prior = depo->prior();
    if (!prior) {
        return;
    }
    push_depo(dp, prior, gen + 1, dp.size());
}
static depos_with_prior flatten_depos(std::vector<WireCell::IDepo::pointer> depos)
{
    depos_with_prior ret;
    for (auto depo : depos) {
        push_depo(ret, depo);
    }
    return ret;
}
#endif

bool Sio::NumpyDepoSaver::operator()(const WireCell::IDepo::pointer& indepo, WireCell::IDepo::pointer& outdepo)
{
    if (indepo) {
        outdepo = indepo;
        m_depos.push_back(indepo);
        return true;
    }
    outdepo = nullptr;

    const size_t ndepos = m_depos.size();
    if (!ndepos) {
        log->warn("NumpyDepoSaver: warning: EOS at {} and no depos seen", m_save_count);
        return true;
    }

#if 0                           // moved to DepoTools
    auto fdepos = flatten_depos(m_depos);
    const size_t nfdepos = fdepos.size();

    /// Dimensions are such that we have a vecotr of N-tuples
    // time, charge, x, y, z, dlong, dtran
    const size_t ndata = 7;
    Array::array_xxf data(nfdepos, ndata);
    // ID, pdg, gen, child
    const size_t ninfo = 4;
    Array::array_xxi info(nfdepos, ninfo);
    for (size_t idepo = 0; idepo != nfdepos; ++idepo) {
        auto depogc = fdepos[idepo];
        auto depo = std::get<0>(depogc);
        auto gen = std::get<1>(depogc);
        auto child = std::get<2>(depogc);
        data(idepo, 0) = depo->time();
        data(idepo, 1) = depo->charge();
        data(idepo, 2) = depo->pos().x();
        data(idepo, 3) = depo->pos().y();
        data(idepo, 4) = depo->pos().z();
        data(idepo, 5) = depo->extent_long();
        data(idepo, 6) = depo->extent_tran();
        info(idepo, 0) = depo->id();
        info(idepo, 1) = depo->pdg();
        info(idepo, 2) = gen;
        info(idepo, 3) = child;
    }
#else

    Array::array_xxf data;
    Array::array_xxi info;
    Aux::fill(data, info, m_depos);
#endif
    log->debug("save ndepos={} at call={}", data.rows(), m_save_count);
    const std::string data_name = String::format("depo_data_%d", m_save_count);
    const std::string info_name = String::format("depo_info_%d", m_save_count);

    const std::string fname = m_filename;
    const std::string mode = "a";

    save2d(data, data_name, fname, mode);
    save2d(info, info_name, fname, mode);
    
    m_depos.clear();

    ++m_save_count;
    return true;
}
