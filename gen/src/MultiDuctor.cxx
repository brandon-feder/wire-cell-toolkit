#include "WireCellGen/MultiDuctor.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Pimpos.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/Exceptions.h"

#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include "WireCellAux/FrameTools.h"

#include <vector>

WIRECELL_FACTORY(MultiDuctor, WireCell::Gen::MultiDuctor, WireCell::IDuctor, WireCell::IConfigurable)

using namespace WireCell;

Gen::MultiDuctor::MultiDuctor(const std::string anode)
  : m_anode_tn(anode)
  , m_tick(0.5 * units::us)
  , m_start_time(0.0 * units::ns)
  , m_readout_time(5.0 * units::ms)
  , m_frame_count(0)
  , m_continuous(false)
  , m_eos(true)
{
}
Gen::MultiDuctor::~MultiDuctor() {}

WireCell::Configuration Gen::MultiDuctor::default_configuration() const
{
    // The "chain" is a list of dictionaries.  Each provides:
    // - ductor: TYPE[:NAME] of a ductor component
    // - rule: name of a rule function to apply
    // - args: args for the rule function
    //
    // Rule functions:
    // wirebounds (list of wire ranges)
    // bool (true/false)

    Configuration cfg;
    cfg["anode"] = m_anode_tn;
    cfg["chain"] = Json::arrayValue;

    // must be consistent with subductors
    cfg["tick"] = m_tick;

    /// The initial time for this ductor
    cfg["start_time"] = m_start_time;

    /// The time span for each readout.
    cfg["readout_time"] = m_readout_time;

    /// If false then determine start time of each readout based on the
    /// input depos.  This option is useful when running WCT sim on a
    /// source of depos which have already been "chunked" in time.  If
    /// true then this Ductor will continuously simulate all time in
    /// "readout_time" frames leading to empty frames in the case of
    /// some readout time with no depos.
    cfg["continuous"] = m_continuous;

    /// Allow for a custom starting frame number
    cfg["first_frame_number"] = m_frame_count;

    return cfg;
}

struct Wirebounds {
    std::vector<const Pimpos*> pimpos;
    Json::Value jargs;
    Wirebounds(const std::vector<const Pimpos*>& p, Json::Value jargs)
      : pimpos(p)
      , jargs(jargs)
    {
    }

    bool operator()(IDepo::pointer depo)
    {
        if (!depo) {
            std::cerr << "Gen::MultiDuctor::Wirebounds: error: no depo given\n";
            return false;
        }

        // fixme: it's possibly really slow to do all this Json::Value
        // access deep inside this loop.  If so, the Json::Values
        // should be run through once in configure() and stored into
        // some faster data structure.

        // return true if depo is "in" ANY region.
        for (auto jregion : jargs) {
            bool inregion = true;

            // depo must be in ALL ranges of a region
            for (auto jrange : jregion) {
                int iplane = jrange["plane"].asInt();
                int imin = jrange["min"].asInt();
                int imax = jrange["max"].asInt();

                // double drift = pimpos[iplane]->distance(depo->pos(), 0);
                double pitch = pimpos[iplane]->distance(depo->pos(), 2);
                int iwire = pimpos[iplane]->region_binning().bin(pitch);  // fixme: warning: round off error?
                inregion = inregion && (imin <= iwire && iwire <= imax);
                if (!inregion) {
                    // std::cerr << "Wirebounds: wire: "<<iwire<<" of plane " << iplane << " not in [" << imin << "," <<
                    // imax << "]\n";
                    break;  // not in this view's region, no reason to keep checking.
                }
            }
            if (inregion) {  // only need one region
                return true;
            }
        }
        return false;
    }
};

struct ReturnBool {
    bool ok;
    ReturnBool(Json::Value jargs)
      : ok(jargs.asBool())
    {
    }
    bool operator()(IDepo::pointer depo)
    {
        if (!depo) {
            return false;
        }
        return ok;
    }
};

void Gen::MultiDuctor::configure(const WireCell::Configuration& cfg)
{
    m_readout_time = get<double>(cfg, "readout_time", m_readout_time);
    m_tick = get<double>(cfg, "tick", m_tick);
    m_start_time = get<double>(cfg, "start_time", m_start_time);
    m_frame_count = get<int>(cfg, "first_frame_number", m_frame_count);
    m_continuous = get(cfg, "continuous", m_continuous);

    m_anode_tn = get(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);
    if (!m_anode) {
        std::cerr << "Gen::MultiDuctor::configure: error: unknown anode: " << m_anode_tn << std::endl;
        return;
    }
    auto jchains = cfg["chains"];
    if (jchains.isNull()) {
        std::cerr << "Gen::MultiDuctor::configure: warning: configured with empty collection of chains\n";
        return;
    }

    /// fixme: this is totally going to break when going to two-faced anodes.
    if (m_anode->faces().size() > 1) {
        std::cerr << "Gen::MultDuctor:configure: warning: I currently only support a front-faced AnodePlane.\n";
    }

    std::vector<const Pimpos*> pimpos;
    for (auto face : m_anode->faces()) {
        if (face->planes().empty()) {
            std::cerr << "Gen::MultDuctor: not given multi-plane AnodeFace for face " << face->ident() << "\n";
            continue;
        }
        for (auto plane : face->planes()) {
            pimpos.push_back(plane->pimpos());
        }
        break;  // fixme:
    }
    if (pimpos.size() != 3) {
        std::cerr << "Gen::MultiDuctor got unexpected number planes (" << pimpos.size() << ") from anode\n";
        THROW(ValueError() << errmsg{"Gen::MultiDuctor got unexpected number planes"});
    }

    for (auto jchain : jchains) {
        std::cerr << "Gen::MultiDuctor::configure chain:\n";
        ductorchain_t dchain;

        for (auto jrule : jchain) {
            auto rule = jrule["rule"].asString();
            auto ductor_tn = jrule["ductor"].asString();
            std::cerr << "\tMultiDuctor: " << ductor_tn << " rule: " << rule << std::endl;
            auto ductor = Factory::find_tn<IDuctor>(ductor_tn);
            if (!ductor) {
                THROW(KeyError() << errmsg{"Failed to find (sub) Ductor: " + ductor_tn});
            }
            auto jargs = jrule["args"];
            if (rule == "wirebounds") {
                dchain.push_back(SubDuctor(ductor_tn, Wirebounds(pimpos, jargs), ductor));
            }
            if (rule == "bool") {
                dchain.push_back(SubDuctor(ductor_tn, ReturnBool(jargs), ductor));
            }
        }
        m_chains.push_back(dchain);
    }  // loop to store chains of ductors
}

// void Gen::MultiDuctor::reset()
// {
//     // forward message
//     for (auto& chain : m_chains) {
//         for (auto& sd : chain) {
//             sd.ductor->reset();
//         }
//     }
// }

// Return true if ready to start processing and capture start time if
// in continuous mode.
bool Gen::MultiDuctor::start_processing(const input_pointer& depo)
{
    if (!depo) {
        m_eos = true;
        return true;
    }
    if (!m_continuous) {
        if (depo && m_eos) {
            m_eos = false;
            m_start_time = depo->time();
            return false;
        }
    }
    return depo->time() > m_start_time + m_readout_time;
}

void Gen::MultiDuctor::dump_frame(const IFrame::pointer frame, std::string msg)
{
    auto traces = frame->traces();
    if (!traces or traces->empty()) {
        std::cerr << msg << " fid:" << frame->ident() << " has no traces\n";
        return;
    }

    auto mm = Aux::tbin_range(*traces);
    std::cerr << msg << " fid:" << frame->ident() << " #ch:" << traces->size() << " t:" << std::setprecision(12)
              << m_start_time / units::us << "us "
              << " tbins:[" << mm.first << "," << mm.second << "]\n";
}

// Outframes should not get a terminating nullptr even if depo is nullptr
void Gen::MultiDuctor::maybe_extract(const input_pointer& depo, output_queue& outframes)
{
    if (!start_processing(depo)) {  // may set m_start_time
        return;
    }

    // Must flush all sub-ductors to assure synchronization.
    for (auto& chain : m_chains) {
        for (auto& sd : chain) {
            output_queue newframes;
            (*sd.ductor)(nullptr, newframes);  // flush with EOS marker
            merge(newframes);                  // updates frame buffer
        }
    }

    // we must read out, and yet we have nothing
    //
    // fixme: the default behavior of this is to make sequential
    // blocks of m_readout_time even if there be no data.  Need to
    // also handle a more isolated running where the readout window is
    // data driven up until EOS is found.
    if (!m_frame_buffer.size()) {
        std::cerr << "MultiDuctor: returning empty frame with " << m_frame_count << " at " << m_start_time << "\n";

        ITrace::vector traces;
        auto frame = std::make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, m_tick);
        outframes.push_back(frame);

        m_start_time += m_readout_time;
        ++m_frame_count;
        return;
    }

    // At this point the m_frame_buffer has at least some frames from
    // multiple sub-ductors, each of different time and channel extent
    // and potentially out of order.

    const double target_time = m_start_time + m_readout_time;
    output_queue to_keep, to_extract;
    for (auto frame : m_frame_buffer) {
        if (!frame) {
            std::cerr << "Gen::MultiDuctor: skipping null frame in frame buffer\n";
            continue;
        }
        if (!frame->traces()) {
            std::cerr << "Gen::MultiDuctor: skipping empty frame in frame buffer\n";
            continue;
        }

        {
            const double tick = frame->tick();
            if (std::abs(tick - m_tick) > 0.0001) {
                std::cerr << "MultiDuctor: configuration error: got different tick in frame from sub-ductor = "
                          << tick / units::us << "us, mine = " << m_tick / units::us << "us\n";
                THROW(ValueError() << errmsg{"tick size mismatch"});
            }
        }

        int cmp = Aux::frmtcmp(frame, target_time);
        // std::cerr << "Gen::MultiDuctor: checking to keep: "
        //           << std::setprecision(12)
        //           << "t_target=" << target_time/units::us << "us, "
        //           << "cmp returns " << cmp << ", frame is:\n";
        // dump_frame(frame, "\t");

        if (cmp < 0) {
            to_extract.push_back(frame);
            continue;
        }
        if (cmp > 0) {
            to_keep.push_back(frame);
            continue;
        }

        // If cmp==0 above then both halves of the pair should hold a frame.
        auto ff = Aux::split(frame, target_time);
        if (ff.first) {
            to_extract.push_back(ff.first);
            // dump_frame(ff.first, "Gen::MultiDuctor: to extract");
        }
        else {
            std::cerr << "Gen::MultiDuctor: error: early split is null\n";
        }

        if (ff.second) {
            to_keep.push_back(ff.second);
            // dump_frame(ff.second, "Gen::MultiDuctor: to keep");
        }
        else {
            std::cerr << "Gen::MultiDuctor: error: late split is null\n";
        }
    }
    m_frame_buffer = to_keep;

    if (!to_extract.size()) {
        // we read out, and yet we have nothing

        std::cerr << "MultiDuctor: returning empty frame after sub frame sorting with " << m_frame_count << " at "
                  << m_start_time << "\n";

        ITrace::vector traces;
        auto frame = std::make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, m_tick);
        outframes.push_back(frame);
        m_start_time += m_readout_time;
        ++m_frame_count;
        return;
    }

    /// Big fat lazy programmer FIXME: there is a bug waiting to bite
    /// here.  If somehow a sub-ductor manages to make a frame that
    /// remains bigger than m_readout_time after the split() above, it
    /// will cause the final output frame to extend past requested
    /// duration.  There needs to be a loop over to_extract added.

    /// Big fat lazy programmer FIXME v2: there can be multiple traces
    /// on the same channel which may overlap in time.

    ITrace::vector traces;
    for (auto frame : to_extract) {
        const double tref = frame->time();
        const int extra_tbins = (tref - m_start_time) / m_tick;
        for (auto trace : (*frame->traces())) {
            const int tbin = trace->tbin() + extra_tbins;
            const int chid = trace->channel();
            // std::cerr <<traces.size()
            //           <<" tstart="<<m_start_time/units::us<<"us"
            //           <<" tref="<<tref/units::us << "us"
            //           <<" tbin="<<tbin
            //           <<" extra bins="<< extra_tbins<<" chid="<<chid<<"\n";
            auto mtrace = std::make_shared<SimpleTrace>(chid, tbin, trace->charge());
            traces.push_back(mtrace);
        }
    }
    auto frame = std::make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, m_tick);
    dump_frame(frame, "Gen::MultiDuctor: output frame");

    outframes.push_back(frame);
    m_start_time += m_readout_time;
    ++m_frame_count;
}

void Gen::MultiDuctor::merge(const output_queue& newframes)
{
    for (auto frame : newframes) {
        if (!frame) {
            continue;
        }  // skip internal EOS
        auto traces = frame->traces();
        if (!traces) {
            continue;
        }
        if (traces->empty()) {
            continue;
        }

        // dump_frame(frame, "Gen::MultiDuctor: merging frame");
        m_frame_buffer.push_back(frame);
    }
}

bool Gen::MultiDuctor::operator()(const input_pointer& depo, output_queue& outframes)
{
    // end of stream processing
    if (!depo) {
        // std::cerr << "Gen::MultiDuctor: end of stream processing\n";
        maybe_extract(depo, outframes);
        outframes.push_back(nullptr);  // pass on EOS marker
        if (!m_frame_buffer.empty()) {
            std::cerr << "Gen::MultiDuctor: purging " << m_frame_buffer.size() << " frames at EOS\n";
            for (auto frame : m_frame_buffer) {
                dump_frame(frame, "\t");
            }
            m_frame_buffer.clear();
        }
        return true;
    }

    // check each rule in the chain to find match
    bool all_okay = true;
    int count = 0;
    for (auto& chain : m_chains) {
        for (auto& sd : chain) {
            if (!sd.check(depo)) {
                continue;
            }

            // found a matching sub-ductor
            ++count;

            // Normal buffered processing.
            output_queue newframes;
            bool ok = (*sd.ductor)(depo, newframes);
            merge(newframes);
            all_okay = all_okay && ok;

            // got a match so abandon chain
            break;
        }
    }
    if (count == 0) {
        std::cerr << "Gen::MultiDuctor: warning: no appropriate Ductor for depo at: " << depo->pos() << std::endl;
    }

    maybe_extract(depo, outframes);
    if (outframes.size()) {
        std::cerr << "Gen::MultiDuctor: returning " << outframes.size() << " frames\n";
    }
    return true;
}
