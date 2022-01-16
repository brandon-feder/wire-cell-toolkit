#include "WireCellSio/DepoFileSource.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Stream.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/NamedFactory.h"

#include "WireCellIface/SimpleDepo.h"
#include "WireCellIface/SimpleDepoSet.h"

#include "WireCellAux/DepoTools.h"

WIRECELL_FACTORY(DepoFileSource, WireCell::Sio::DepoFileSource,
                 WireCell::INamed,
                 WireCell::IDepoSetSource, WireCell::IConfigurable)

using namespace WireCell;
using namespace WireCell::Stream;
using namespace WireCell::String;

Sio::DepoFileSource::DepoFileSource()
    : Aux::Logger("DepoFileSource", "io")
{
}

Sio::DepoFileSource::~DepoFileSource()
{
}

WireCell::Configuration Sio::DepoFileSource::default_configuration() const
{
    Configuration cfg;
    // Input tar stream
    cfg["inname"] = m_inname;

    // This will be multiplied to the "charge" of each depo.  Eg, it
    // can be usfully set to -1 to accomodate diffierent conventions
    // for electron charge sign.
    cfg["scale"] = m_scale;
    return cfg;
}

void Sio::DepoFileSource::configure(const WireCell::Configuration& cfg)
{
    m_inname = get(cfg, "inname", m_inname);
    m_scale = get(cfg, "scale", m_scale);

    m_in.clear();
    input_filters(m_in, m_inname);
    if (m_in.size() < 2) {     // must have at least get tar filter + file source.
        THROW(ValueError() << errmsg{"DepoFielSource: unsupported inname: " + m_inname});
    }

    log->debug("reading {} with scale={}", m_inname, m_scale);
}

// ident, type, tag, ext
struct filemd_t {
    std::string fname{""};
    bool okay{false};
    int ident{0};
    std::string type{""}, tag{""}, ext{""};
    // "depo", "data|info", "npy
};

static
filemd_t parse_filename(const std::string& fname)
{
    if (fname.empty()) { return {fname}; }
    auto parts = split(fname, "_");
    if (parts.size() != 3) { return {fname}; }
    auto rparts = split(parts[2], ".");
    int ident = std::atoi(rparts[0].c_str());
    return filemd_t{fname, true, ident, parts[0], parts[1], rparts[1]};
}

static
filemd_t read_pig(std::istream& si, pigenc::File& pig)
{
    std::string fname{""};
    size_t fsize{0};
    custard::read(si, fname, fsize);
    pig.read(si);
    return parse_filename(fname);
}

// Read one file set from tar stream and save to fraem if tag matches
IDepoSet::pointer Sio::DepoFileSource::next()
{
    // Full eager load of next two files
    pigenc::File pig1, pig2;
    auto fmd1 = read_pig(m_in, pig1);
    if (m_in.eof()) {
        log->debug("call={}, read1 depo stream EOF with file={}", m_count, m_inname);
        return nullptr;
    }
    auto fmd2 = read_pig(m_in, pig2);
    if (m_in.eof()) {
        log->debug("call={}, read2 depo stream EOF with file={}", m_count, m_inname);
        return nullptr;
    }
    if (!m_in) {
        log->error("call={}, depo read fail with file={}", m_count, m_inname);
        return nullptr;
    }
    if (!fmd1.okay) {
        log->error("call={}, depo read fail to parse npy file name {} in file={}", m_count, fmd1.fname, m_inname);
        return nullptr;
    }
    if (!fmd2.okay) {
        log->error("call={}, depo read fail to parse npy file name {} in file={}", m_count, fmd2.fname, m_inname);
        return nullptr;
    }
    if (fmd1.ident != fmd2.ident) {
        log->error("call={}, ident mismatch {} != {} in file={}",
                   m_count, m_count, fmd1.ident, fmd2.ident);
        return nullptr;
    }
    const int ident = fmd1.ident;

    // Allow free order of data/info w/in the pair.
    pigenc::File* dpig=&pig1;
    pigenc::File* ipig=&pig2;
    if (fmd2.tag == "data") {
        dpig = &pig2;
        ipig = &pig1;
    }
    /// Note, this emits have non-ascii chars from Numpy header
    // log->debug("call={}, data header: {}", m_count, dpig->header().str());
    // log->debug("call={}, info header: {}", m_count, ipig->header().str());

    using array_xxfrw = Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    array_xxfrw darr;
    using array_xxirw = Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    array_xxirw iarr;

    bool ok = false;

    ok = pigenc::eigen::load(*dpig, darr);
    if (!ok) {
        log->error("call={}, short load failed data array file={}",
                   m_count, m_inname);
        return nullptr;
    }

    ok = pigenc::eigen::load(*ipig, iarr);
    if (!ok) {
        log->error("call={}, short load failed for info array file={}",
                   m_count, m_inname);
        return nullptr;
    }

    if (darr.rows() != iarr.rows()) {
        log->error("call={}, array row mismatch data={} info={}",
                   m_count, darr.rows(), iarr.rows());
        return nullptr;
    }
    const size_t ndepos = darr.rows();
        
    const size_t ndata = 7;
    if (darr.cols() != ndata) {
        log->error("call={}, data column mismatch got={} want={}",
                   m_count, darr.cols(), ndata);
        return nullptr;
    }

    const size_t ninfo = 4; 
    if (darr.cols() != ndata) {
        log->error("call={}, info column mismatch got={} want={}",
                   m_count, iarr.cols(), ninfo);
        return nullptr;
    }
        
    std::vector<SimpleDepo*> sdepos;
    for (size_t ind=0; ind < ndepos; ++ind) {

        auto sdepo = new SimpleDepo(
            darr(ind, 0),        // t
            Point(darr(ind, 2),  // x
                  darr(ind, 3),  // y
                  darr(ind, 4)), // z
            darr(ind, 1)*m_scale,// q
            nullptr,             // prior
            darr(ind, 5),        // extent_long
            darr(ind, 6),        // extent_tran
            iarr(ind, 0),        // id
            iarr(ind, 1));       // pdg

        const auto gen = iarr(ind, 2);
        if (gen > 0) {
            // this depo is a prior
            const size_t other = iarr(ind, 3);
            if (other >= sdepos.size()) {
                log->warn("call={}, prior depo {} not provided in {}",
                          m_count, other, sdepos.size());
            }
            else {
                auto idepo = IDepo::pointer(sdepo);
                sdepo = nullptr;
                sdepos[other]->set_prior(idepo);
            }
        }
        // We save both gen=0 and gen>0 as nullptrs to preserve indexing
        sdepos.push_back(sdepo);
    }

    WireCell::IDepo::vector depos;
    for (auto sdepo: sdepos) {
        if (sdepo) {
            auto idepo = IDepo::pointer(sdepo);
            sdepo = nullptr;
            depos.push_back(idepo);
        }
    }

    log->debug("call={} loaded {} depos from ident {} stream {}",
               m_count, depos.size(), ident, m_inname);

    return std::make_shared<SimpleDepoSet>(ident, depos);
}

bool Sio::DepoFileSource::operator()(IDepoSet::pointer& ds)
{
    ds = nullptr;
    if (m_eos_sent) {
        ++m_count;
        return false;
    }
    ds = next();
    if (!ds) {
        m_eos_sent = true;
        log->debug("EOS at call={}", m_count);
    }
    ++m_count;
    return true;
}
