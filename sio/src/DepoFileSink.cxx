#include "WireCellSio/DepoFileSink.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/Array.h"
#include "WireCellUtil/Stream.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellAux/DepoTools.h"

WIRECELL_FACTORY(DepoFileSink,
                 WireCell::Sio::DepoFileSink,
                 WireCell::INamed,
                 WireCell::IDepoSetSink,
                 WireCell::ITerminal,
                 WireCell::IConfigurable)

using namespace WireCell;
using namespace WireCell::Stream;

Sio::DepoFileSink::DepoFileSink()
    : Aux::Logger("DepoFileSink", "io")
{
}

Sio::DepoFileSink::~DepoFileSink()
{
}

void Sio::DepoFileSink::finalize()
{
    log->debug("closing {} after {} calls", m_outname, m_count);
    m_out.pop();
}

WireCell::Configuration Sio::DepoFileSink::default_configuration() const
{
    Configuration cfg;
    // Output tar file name.
    cfg["outname"] = m_outname;
    return cfg;
}

void Sio::DepoFileSink::configure(const WireCell::Configuration& cfg)
{
    m_outname = get(cfg, "outname", m_outname);

    m_out.clear();
    output_filters(m_out, m_outname);
    if (m_out.size() < 2) {
        // must have at least get tar filter + file sink.
        THROW(ValueError()
              << errmsg{"DepoFileSink: unsupported outname: "
                  + m_outname});
    }
}

bool Sio::DepoFileSink::operator()(const IDepoSet::pointer& deposet)
{
    if (!deposet) { // eos
        log->debug("EOS at call={}", m_count);
        ++m_count;
        return true;
    }

    Array::array_xxf data;
    Array::array_xxi info;
    Aux::fill(data, info, *deposet->depos().get());

    log->debug("Writing {} depos at call={}", data.rows(), m_count);

    const std::string dname = String::format("depo_data_%d.npy",
                                             deposet->ident());
    write(m_out, dname, data);
    
    const std::string iname = String::format("depo_info_%d.npy",
                                             deposet->ident());
    write(m_out, iname, info);
    
    m_out.flush();

    ++m_count;
    return true;
}
