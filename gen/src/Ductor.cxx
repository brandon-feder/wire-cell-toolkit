#include "WireCellGen/Ductor.h"
#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellGen/ImpactZipper.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleTrace.h"
#include "WireCellIface/SimpleFrame.h"

#include <string>

WIRECELL_FACTORY(Ductor, WireCell::Gen::Ductor, WireCell::IDuctor, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::Ductor::Ductor()
  : m_anode_tn("AnodePlane")
  , m_rng_tn("Random")
  , m_start_time(0.0 * units::ns)
  , m_readout_time(5.0 * units::ms)
  , m_tick(0.5 * units::us)
  , m_drift_speed(1.0 * units::mm / units::us)
  , m_nsigma(3.0)
  , m_fluctuate(true)
  , m_mode("continuous")
  , m_frame_count(0)
  , l(Log::logger("sim"))
{
}

WireCell::Configuration Gen::Ductor::default_configuration() const
{
    Configuration cfg;

    /// How many Gaussian sigma due to diffusion to keep before truncating.
    put(cfg, "nsigma", m_nsigma);

    /// Whether to fluctuate the final Gaussian deposition.
    put(cfg, "fluctuate", m_fluctuate);

    /// The initial time for this ductor
    put(cfg, "start_time", m_start_time);

    /// The time span for each readout.
    put(cfg, "readout_time", m_readout_time);

    /// The sample period
    put(cfg, "tick", m_tick);

    /// If false then determine start time of each readout based on the
    /// input depos.  This option is useful when running WCT sim on a
    /// source of depos which have already been "chunked" in time.  If
    /// true then this Ductor will continuously simulate all time in
    /// "readout_time" frames leading to empty frames in the case of
    /// some readout time with no depos.
    put(cfg, "continuous", true);

    /// Fixed mode simply reads out the same time window all the time.
    /// It implies discontinuous (continuous == false).
    put(cfg, "fixed", false);

    /// The nominal speed of drifting electrons
    put(cfg, "drift_speed", m_drift_speed);

    /// Allow for a custom starting frame number
    put(cfg, "first_frame_number", m_frame_count);

    /// Name of component providing the anode plane.
    put(cfg, "anode", m_anode_tn);
    put(cfg, "rng", m_rng_tn);

    cfg["pirs"] = Json::arrayValue;
    /// don't set here so user must, but eg:
    // cfg["pirs"][0] = "PlaneImpactResponseU";
    // cfg["pirs"][1] = "PlaneImpactResponseV";
    // cfg["pirs"][2] = "PlaneImpactResponseW";

    // Tag to use for frame and traces will get this tag + the anode
    // ID.
    cfg["tag"] = "ductor";

    return cfg;
}

void Gen::Ductor::configure(const WireCell::Configuration& cfg)
{
    m_anode_tn = get<string>(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);

    m_nsigma = get<double>(cfg, "nsigma", m_nsigma);
    bool continuous = get<bool>(cfg, "continuous", true);
    bool fixed = get<bool>(cfg, "fixed", false);

    m_mode = "continuous";
    if (fixed) {
        m_mode = "fixed";
    }
    else if (!continuous) {
        m_mode = "discontinuous";
    }

    m_fluctuate = get<bool>(cfg, "fluctuate", m_fluctuate);
    m_rng = nullptr;
    if (m_fluctuate) {
        m_rng_tn = get(cfg, "rng", m_rng_tn);
        m_rng = Factory::find_tn<IRandom>(m_rng_tn);
    }

    m_readout_time = get<double>(cfg, "readout_time", m_readout_time);
    m_tick = get<double>(cfg, "tick", m_tick);
    m_start_time = get<double>(cfg, "start_time", m_start_time);
    m_drift_speed = get<double>(cfg, "drift_speed", m_drift_speed);
    m_frame_count = get<int>(cfg, "first_frame_number", m_frame_count);

    auto jpirs = cfg["pirs"];
    if (jpirs.isNull() or jpirs.empty()) {
        l->critical("must configure with some plane impace response components");
        THROW(ValueError() << errmsg{"Gen::Ductor: must configure with some plane impact response components"});
    }
    m_pirs.clear();
    for (auto jpir : jpirs) {
        auto tn = jpir.asString();
        auto pir = Factory::find_tn<IPlaneImpactResponse>(tn);
        m_pirs.push_back(pir);
    }

    m_tag = get<std::string>(cfg, "tag", "ductor");

    l->debug("Ductor tagging {}, AnodePlane: {}, mode: {}, fluctuate: {}, time start: {} ms, readout time: {} ms, frame start: {}",
             m_tag,
             m_anode_tn, m_mode, (m_fluctuate ? "on" : "off"), m_start_time / units::ms, m_readout_time / units::ms,
             m_frame_count);
}

ITrace::vector Gen::Ductor::process_face(IAnodeFace::pointer face, const IDepo::vector& face_depos)
{
    ITrace::vector traces;

    int iplane = -1;
    for (auto plane : face->planes()) {
        ++iplane;

        const Pimpos* pimpos = plane->pimpos();

        Binning tbins(m_readout_time / m_tick, m_start_time, m_start_time + m_readout_time);

        Gen::BinnedDiffusion bindiff(*pimpos, tbins, m_nsigma, m_rng);
        for (auto depo : face_depos) {
            bindiff.add(depo, depo->extent_long() / m_drift_speed, depo->extent_tran());
        }

        auto& wires = plane->wires();

        auto pir = m_pirs.at(iplane);
        Gen::ImpactZipper zipper(pir, bindiff);

        const int nwires = pimpos->region_binning().nbins();
        for (int iwire = 0; iwire < nwires; ++iwire) {
            auto wave = zipper.waveform(iwire);

            auto mm = Waveform::edge(wave);
            if (mm.first == (int) wave.size()) {  // all zero
                continue;
            }

            int chid = wires[iwire]->channel();
            int tbin = mm.first;

            ITrace::ChargeSequence charge(wave.begin() + mm.first, wave.begin() + mm.second);
            auto trace = make_shared<SimpleTrace>(chid, tbin, charge);
            traces.push_back(trace);
        }
    }
    return traces;
}

void Gen::Ductor::process(output_queue& frames)
{
    ITrace::vector traces;

    for (auto face : m_anode->faces()) {
        // Select the depos which are in this face's sensitive volume
        IDepo::vector face_depos, dropped_depos;
        auto bb = face->sensitive();
        if (bb.empty()) {
            l->debug("anode: {} face: {} is marked insensitive, skipping", m_anode->ident(), face->ident());
            continue;
        }

        for (auto depo : m_depos) {
            if (bb.inside(depo->pos())) {
                face_depos.push_back(depo);
            }
            else {
                dropped_depos.push_back(depo);
            }
        }

        if (face_depos.size()) {
            auto ray = bb.bounds();
            l->debug(
                "anode: {}, face: {}, processing {} depos spanning "
                "t:[{},{}]ms, bb:[{}-->{}]cm",
                m_anode->ident(), face->ident(), face_depos.size(), face_depos.front()->time() / units::ms,
                face_depos.back()->time() / units::ms, ray.first / units::cm, ray.second / units::cm);
        }
        if (dropped_depos.size()) {
            auto ray = bb.bounds();
            l->debug(
                "anode: {}, face: {}, dropped {} depos spanning "
                "t:[{},{}]ms, outside bb:[{}-->{}]cm",
                m_anode->ident(), face->ident(), dropped_depos.size(), dropped_depos.front()->time() / units::ms,
                dropped_depos.back()->time() / units::ms, ray.first / units::cm, ray.second / units::cm);
        }

        auto newtraces = process_face(face, face_depos);
        traces.insert(traces.end(), newtraces.begin(), newtraces.end());
    }

    auto frame = make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, m_tick);
    IFrame::trace_list_t indices(traces.size());
    for (size_t ind = 0; ind < traces.size(); ++ind) {
        indices[ind] = ind;
    }
    frame->tag_traces(m_tag + std::to_string(m_anode->ident()), indices);
    frame->tag_frame(m_tag);
    frames.push_back(frame);
    l->debug("made frame: {} with {} traces @ {}ms", m_frame_count, traces.size(), m_start_time / units::ms);

    // fixme: what about frame overflow here?  If the depos extend
    // beyond the readout where does their info go?  2nd order,
    // diffusion and finite field response can cause depos near the
    // end of the readout to have some portion of their waveforms
    // lost?
    m_depos.clear();

    if (m_mode == "continuous") {
        m_start_time += m_readout_time;
    }

    ++m_frame_count;
}

// Return true if ready to start processing and capture start time if
// in continuous mode.
bool Gen::Ductor::start_processing(const input_pointer& depo)
{
    if (!depo) {
        return true;
    }

    if (m_mode == "fixed") {
        // fixed mode waits until EOS
        return false;
    }

    if (m_mode == "discontinuous") {
        // discontinuous mode sets start time on first depo.
        if (m_depos.empty()) {
            m_start_time = depo->time();
            return false;
        }
    }

    // continuous and discontinuous modes follow Just Enough
    // Processing(TM) strategy.

    // Note: we use this depo time even if it may not actually be
    // inside our sensitive volume.
    bool ok = depo->time() > m_start_time + m_readout_time;
    return ok;
}

bool Gen::Ductor::operator()(const input_pointer& depo, output_queue& frames)
{
    if (start_processing(depo)) {
        process(frames);
    }

    if (depo) {
        m_depos.push_back(depo);
    }
    else {
        frames.push_back(nullptr);
    }

    return true;
}
