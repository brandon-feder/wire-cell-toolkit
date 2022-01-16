#include "WireCellHio/HDF5FrameTap.h"
#include "WireCellIface/ITrace.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include "WireCellAux/FrameTools.h"

#include "WireCellUtil/NamedFactory.h"

#include "WireCellHio/Util.h"

#include <string>
#include <vector>

/// macro to register name - concrete pair in the NamedFactory
/// @param NAME - used to configure node in JSON/Jsonnet
/// @parame CONCRETE - C++ concrete type
/// @parame ... - interfaces
WIRECELL_FACTORY(HDF5FrameTap, WireCell::Hio::HDF5FrameTap, WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace WireCell;

Hio::HDF5FrameTap::HDF5FrameTap()
  : m_save_count(0)
  , l(Log::logger("io"))
{
}

Hio::HDF5FrameTap::~HDF5FrameTap() {}

void Hio::HDF5FrameTap::configure(const WireCell::Configuration &cfg)
{
    auto anode_tn = cfg["anode"].asString();
    m_anode = Factory::find_tn<IAnodePlane>(anode_tn);

    m_cfg = cfg;

    std::string fn = cfg["filename"].asString();
    if (fn.empty()) {
        THROW(ValueError() << errmsg{"Must provide output filename to HDF5FrameTap"});
    }

    h5::create(fn, H5F_ACC_TRUNC);
}

WireCell::Configuration Hio::HDF5FrameTap::default_configuration() const
{
    Configuration cfg;

    // If digitize is true, then samples as 16 bit ints.  Otherwise
    // save as 32 bit floats.
    cfg["digitize"] = false;

    // This number is set to the waveform sample array before any
    // charge is added.
    cfg["baseline"] = 0.0;

    // This number will be multiplied to each waveform sample before
    // casting to dtype.
    cfg["scale"] = 1.0;

    // This number will be added to each scaled waveform sample before
    // casting to dtype.
    cfg["offset"] = 0.0;

    cfg["anode"] = "AnodePlane";
    cfg["nticks"] = 6000;
    cfg["tick0"] = 0;

    // The frame tags to consider for saving.  If null or empty then all traces are used.
    cfg["trace_tags"] = Json::arrayValue;
    // The summary tags to consider for saving
    // cfg["summary_tags"] = Json::arrayValue;
    // The channel mask maps to consider for saving
    // cfg["chanmaskmaps"] = Json::arrayValue;

    // The output file name to write.  Only compressed (zipped) Numpy
    // files are supported.  Writing is always in "append" mode.  It's
    // up to the user to delete a previous instance of the file if
    // it's old contents are not wanted.
    cfg["filename"] = "wct-frame.hdf5";

    cfg["chunk"] = Json::arrayValue;
    cfg["gzpi"] = 9;
    cfg["high_throughput"] = true;

    return cfg;
}

bool Hio::HDF5FrameTap::operator()(const IFrame::pointer &inframe, IFrame::pointer &outframe)
{
    std::lock_guard<std::mutex> guard(g_h5cpp_mutex);
    if (!inframe) {
        l->debug("HDF5FrameTap: EOS");
        outframe = nullptr;
        return true;
    }

    const int tick0 = m_cfg["tick0"].asInt();
    const int nticks = m_cfg["nticks"].asInt();
    const int tbeg = tick0;
    const int tend = tick0 + nticks - 1;
    auto channels = m_anode->channels();
    const int cbeg = channels.front();
    const int cend = channels.back();
    l->debug("{}: t: {} - {}; c: {} - {}", m_cfg["anode"].asString(), tbeg, tend, cbeg, cend);

    outframe = inframe;  // pass through actual frame

    const std::string mode = "a";

    const float baseline = m_cfg["baseline"].asFloat();
    const float scale = m_cfg["scale"].asFloat();
    const float offset = m_cfg["offset"].asFloat();
    const bool digitize = m_cfg["digitize"].asBool();

    const std::string fname = m_cfg["filename"].asString();

    // Eigen3 array is indexed as (irow, icol) or (ichan, itick)
    // one row is one channel, one column is a tick.
    // Numpy saves reversed dimensions: {ncols, nrows} aka {ntick, nchan} dimensions.

    if (m_cfg["trace_tags"].isNull() or m_cfg["trace_tags"].empty()) {
        m_cfg["trace_tags"][0] = "";
    }

    if (m_cfg["chunk"].isNull() or m_cfg["chunk"].size() != 2) {
        m_cfg["chunk"].resize(2);
        m_cfg["chunk"][0] = 0;
        m_cfg["chunk"][1] = 0;
    }

    size_t chunk_ncols = m_cfg["chunk"][0].asInt();
    size_t chunk_nrows = m_cfg["chunk"][1].asInt();
    l->debug("HDF5FrameTap: chunking ncols={} nrows={}", chunk_ncols, chunk_nrows);

    int gzip_level = m_cfg["gzip"].asInt();
    if (gzip_level < 0 or gzip_level > 9) gzip_level = 9;
    l->debug("HDF5FrameTap: gzip_level {}", gzip_level);

    const bool high_throughput = m_cfg["high_throughput"].asBool();

    std::stringstream ss;
    ss << "HDF5FrameTap: see frame #" << inframe->ident() << " with " << inframe->traces()->size()
       << " traces with frame tags:";
    for (auto t : inframe->frame_tags()) {
        ss << " \"" << t << "\"";
    }
    ss << " and trace tags:";
    for (auto t : inframe->trace_tags()) {
        ss << " \"" << t << "\"";
    }
    ss << " looking for tags:";
    for (auto jt : m_cfg["trace_tags"]) {
        ss << " \"" << jt.asString() << "\"";
    }
    l->debug(ss.str());

    h5::fd_t fd = h5::open(m_cfg["filename"].asString(), H5F_ACC_RDWR);
    // TODO some protection

    for (auto jtag : m_cfg["trace_tags"]) {
        const std::string tag = jtag.asString();
        auto traces = Aux::tagged_traces(inframe, tag);
        l->debug("HDF5FrameTap: save {} tagged as {}", traces.size(), tag);
        if (traces.empty()) {
            l->warn("HDF5FrameTap: no traces for tag: \"{}\"", tag);
            continue;
        }

        // auto channels = Aux::channels(traces);
        // std::sort(channels.begin(), channels.end());
        // auto chmin = channels.front();
        // auto chmax = channels.back();
        // channels.resize(chmax-chmin+1);
        // std::iota(std::begin(channels), std::end(channels), chmin);
        // auto chbeg = channels.begin();
        // auto chend = channels.end(); //std::unique(chbeg, channels.end());
        // auto tbinmm = Aux::tbin_range(traces);

        // // fixme: may want to give user some config over tbin range to save.
        // const size_t ncols = tbinmm.second-tbinmm.first;
        // const size_t nrows = std::distance(chbeg, chend);

        const size_t ncols = nticks;
        const size_t nrows = cend - cbeg + 1;

        l->debug("HDF5FrameTap: saving ncols={} nrows={}", ncols, nrows);

        if (chunk_ncols < 1 or chunk_ncols > ncols) chunk_ncols = ncols;
        if (chunk_nrows < 1 or chunk_nrows > nrows) chunk_nrows = nrows;
        l->debug("HDF5FrameTap: chunking ncols={} nrows={}", chunk_ncols, chunk_nrows);

        Array::array_xxf arr = Array::array_xxf::Zero(nrows, ncols) + baseline;
        // Aux::fill(arr, traces, channels.begin(), chend, tbinmm.first);
        Aux::fill(arr, traces, channels.begin(), channels.end(), tick0);
        arr = arr * scale + offset;
        int sequence = inframe->ident();
        {  // the 2D frame array
            const std::string aname = String::format("/%d/frame_%s", sequence, tag.c_str());
            if (digitize) {
                Array::array_xxs sarr = arr.cast<short>();
                const short *sdata = sarr.data();
                // cnpy::npz_save(fname, aname, sdata, {ncols, nrows}, mode);
                if (high_throughput) {
                    h5::write<short>(fd, aname, sdata, h5::count{ncols, nrows},
                                     h5::chunk{chunk_ncols, chunk_nrows} | h5::gzip{gzip_level}, h5::high_throughput);
                }
                else {
                    h5::write<short>(fd, aname, sdata, h5::count{ncols, nrows},
                                     h5::chunk{chunk_ncols, chunk_nrows} | h5::gzip{gzip_level});
                }
            }
            else {
                // cnpy::npz_save(fname, aname, arr.data(), {ncols, nrows}, mode);
                if (high_throughput) {
                    h5::write<float>(fd, aname, arr.data(), h5::count{ncols, nrows},
                                     h5::chunk{chunk_ncols, chunk_nrows} | h5::gzip{gzip_level}, h5::high_throughput);
                }
                else {
                    h5::write<float>(fd, aname, arr.data(), h5::count{ncols, nrows},
                                     h5::chunk{chunk_ncols, chunk_nrows} | h5::gzip{gzip_level});
                }
            }
            l->debug("HDF5FrameTap: saved {} with {} channels {} ticks @t={} ms qtot={}", aname, nrows, ncols,
                     inframe->time() / units::ms, arr.sum());
        }

        {  // the channel array
            const std::string aname = String::format("/%d/channels_%s", sequence, tag.c_str());
            // cnpy::npz_save(fname, aname, channels.data(), {nrows}, mode);
            h5::write<int>(fd, aname, channels.data(), h5::count{nrows});
        }

        {  // the tick array
            const std::string aname = String::format("/%d/tickinfo_%s", sequence, tag.c_str());
            // const std::vector<double> tickinfo{inframe->time(), inframe->tick(), (double)tbinmm.first};
            const std::vector<double> tickinfo{inframe->time(), inframe->tick(), (double) tick0};
            // cnpy::npz_save(fname, aname, tickinfo.data(), {3}, mode);
            h5::write<double>(fd, aname, tickinfo.data(), h5::count{3});
        }
    }

    ++m_save_count;
    return true;
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
