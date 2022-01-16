#include "WireCellImg/BlobSetSync.h"
#include "WireCellIface/SimpleBlob.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(BlobSetSync, WireCell::Img::BlobSetSync,
                 WireCell::INamed,
                 WireCell::IBlobSetFanin, WireCell::IConfigurable)
using namespace WireCell;

Img::BlobSetSync::BlobSetSync()
    : Aux::Logger("BlobSetSync", "glue")
    , m_multiplicity(0)
{
}

Img::BlobSetSync::~BlobSetSync() {}

WireCell::Configuration Img::BlobSetSync::default_configuration() const
{
    Configuration cfg;
    cfg["multiplicity"] = (int) m_multiplicity;
    return cfg;
}

void Img::BlobSetSync::configure(const WireCell::Configuration& cfg)
{
    int m = get<int>(cfg, "multiplicity", (int) m_multiplicity);
    if (m <= 0) {
        THROW(ValueError() << errmsg{"BlobSetSync multiplicity must be positive"});
    }
    m_multiplicity = m;
}

std::vector<std::string> Img::BlobSetSync::input_types()
{
    const std::string tname = std::string(typeid(input_type).name());
    std::vector<std::string> ret(m_multiplicity, tname);
    return ret;
}

bool Img::BlobSetSync::operator()(const input_vector& invec, output_pointer& out)
{
    SimpleBlobSet* sbs = new SimpleBlobSet(0, nullptr);
    out = IBlobSet::pointer(sbs);

    int neos = 0;
    std::stringstream perset;
    for (const auto& ibs : invec) {
        if (!ibs) {
            ++neos;
            break;
        }
        ISlice::pointer newslice = ibs->slice();
        if (!sbs->slice() or sbs->slice()->start() > newslice->start()) {
            sbs->m_slice = newslice;
            sbs->m_ident = newslice->ident();
        }
        for (const auto& iblob : ibs->blobs()) {
            sbs->m_blobs.push_back(iblob);
        }
        perset << " " << ibs->blobs().size();
    }
    perset << " ";
    if (neos) {
        out = nullptr;
        log->debug("EOS");
        return true;
    }
    // we get called a lot so make this a trace level!
    SPDLOG_LOGGER_TRACE(log, "sync'ed {} blobs: {}",
                        sbs->m_blobs.size(), perset.str());
    return true;
}
