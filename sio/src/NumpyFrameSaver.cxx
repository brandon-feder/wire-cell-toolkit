#include "WireCellSio/NumpyFrameSaver.h"

#include "WireCellAux/FrameTools.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/NumpyHelper.h"

#include <string>
#include <vector>
#include <algorithm>
#include <tuple>
#include <sstream>

WIRECELL_FACTORY(NumpyFrameSaver, WireCell::Sio::NumpyFrameSaver, WireCell::IFrameFilter, WireCell::IConfigurable)

using namespace WireCell;
using WireCell::Numpy::save2d;

Sio::NumpyFrameSaver::NumpyFrameSaver()
  : m_save_count(0)
  , l(Log::logger("io"))
{
}

Sio::NumpyFrameSaver::~NumpyFrameSaver() {}

WireCell::Configuration Sio::NumpyFrameSaver::default_configuration() const
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

    // The frame tags to consider for saving.  If null or empty then all traces are used.
    cfg["frame_tags"] = Json::arrayValue;
    // The summary tags to consider for saving
    // cfg["summary_tags"] = Json::arrayValue;
    // The channel mask maps to consider for saving
    // cfg["chanmaskmaps"] = Json::arrayValue;

    // The output file name to write.  Only compressed (zipped) Numpy
    // files are supported.  Writing is always in "append" mode.  It's
    // up to the user to delete a previous instance of the file if
    // it's old contents are not wanted.
    cfg["filename"] = "wct-frame.npz";

    return cfg;
}

void Sio::NumpyFrameSaver::configure(const WireCell::Configuration& config) { m_cfg = config; }

bool Sio::NumpyFrameSaver::operator()(const IFrame::pointer& inframe, IFrame::pointer& outframe)
{
    if (!inframe) {
        l->debug("NumpyFrameSaver: EOS");
        outframe = nullptr;
        return true;
    }

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

    if (m_cfg["frame_tags"].isNull() or m_cfg["frame_tags"].empty()) {
        m_cfg["frame_tags"][0] = "";
    }

    std::stringstream ss;
    ss << "NumpyFrameSaver: see frame #" << inframe->ident() << " with " << inframe->traces()->size()
       << " traces with frame tags:";
    for (auto t : inframe->frame_tags()) {
        ss << " \"" << t << "\"";
    }
    ss << " and trace tags:";
    for (auto t : inframe->trace_tags()) {
        ss << " \"" << t << "\"";
    }
    ss << " looking for tags:";
    for (auto jt : m_cfg["frame_tags"]) {
        ss << " \"" << jt.asString() << "\"";
    }
    l->debug(ss.str());

    for (auto jtag : m_cfg["frame_tags"]) {
        const std::string tag = jtag.asString();
        auto traces = Aux::tagged_traces(inframe, tag);
        l->debug("NumpyFrameSaver: save {} tagged as {}", traces.size(), tag);
        if (traces.empty()) {
            l->warn("NumpyFrameSaver: no traces for tag: \"{}\"", tag);
            continue;
        }
        auto channels = Aux::channels(traces);
        std::sort(channels.begin(), channels.end());
        auto chbeg = channels.begin();
        auto chend = std::unique(chbeg, channels.end());
        auto tbinmm = Aux::tbin_range(traces);

        // fixme: may want to give user some config over tbin range to save.
        const size_t ncols = tbinmm.second - tbinmm.first;
        const size_t nrows = std::distance(chbeg, chend);
        l->debug("NumpyFrameSaver: saving ncols={} nrows={}", ncols, nrows);

        Array::array_xxf arr = Array::array_xxf::Zero(nrows, ncols) + baseline;
        Aux::fill(arr, traces, channels.begin(), chend, tbinmm.first);
        arr = arr * scale + offset;

        {  // the 2D frame array
            const std::string aname = String::format("frame_%s_%d", tag.c_str(), m_save_count);
            if (digitize) {
                Array::array_xxs sarr = arr.cast<short>();
                // const short* sdata = sarr.data();
                // cnpy::npz_save(fname, aname, sdata, {ncols, nrows}, mode);
                save2d(sarr, aname, fname, mode);
            }
            else {
                // cnpy::npz_save(fname, aname, arr.data(), {ncols, nrows}, mode);
                save2d(arr, aname, fname, mode);
            }
            l->debug("NumpyFrameSaver: saved {} with {} channels {} ticks @t={} ms qtot={}", aname, nrows, ncols,
                     inframe->time() / units::ms, arr.sum());
        }

        {  // the channel array
            const std::string aname = String::format("channels_%s_%d", tag.c_str(), m_save_count);
            cnpy::npz_save(fname, aname, channels.data(), {nrows}, mode);
            
        }

        {  // the tick array
            const std::string aname = String::format("tickinfo_%s_%d", tag.c_str(), m_save_count);
            const std::vector<double> tickinfo{inframe->time(), inframe->tick(), (double) tbinmm.first};
            cnpy::npz_save(fname, aname, tickinfo.data(), {3}, mode);
        }
    }

    ++m_save_count;
    return true;
}
