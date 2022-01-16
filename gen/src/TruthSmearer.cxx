#include "WireCellGen/TruthSmearer.h"
#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellGen/ImpactZipper.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleTrace.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellUtil/Waveform.h"

#include <string>

WIRECELL_FACTORY(TruthSmearer, WireCell::Gen::TruthSmearer, WireCell::IDuctor, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

/// based on Gen::Ductor
/// without convolution
/// just smearing of charge depos in time and wire dimenstions
/// (time_smear = 0, wire_smear = 1) means "Charge Truth"
/// otherwires, "Signal Truth" with signal processing residual

Gen::TruthSmearer::TruthSmearer()
  : m_anode_tn("AnodePlane")
  , m_rng_tn("Random")
  , m_start_time(0.0 * units::ns)
  , m_readout_time(5.0 * units::ms)
  , m_tick(0.5 * units::us)
  , m_drift_speed(1.0 * units::mm / units::us)
  , m_time_smear(1.4 * units::us)          // Fig.15 in arXiv:1802.08709
  , m_wire_smear_ind(0.75)                 // Fig. 16 in arXiv: 1802.08709
  , m_wire_smear_col(0.95)                 // Fig. 16 in arXiv: 1802.08709
  , m_smear_response_tn("smear_response")  // interface
  , m_truth_gain(-1.0)
  , m_nsigma(3.0)
  , m_fluctuate(true)
  , m_continuous(true)
  , m_frame_count(0)
{
}

WireCell::Configuration Gen::TruthSmearer::default_configuration() const
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

    /// The nominal speed of drifting electrons
    put(cfg, "drift_speed", m_drift_speed);

    /// Gaussian longitudinal (time) smearing  -- time filter in SP
    put(cfg, "time_smear", m_time_smear);

    /// Discrete wire smear -- wire filter in Fig.16 arXiv: 1802.08709
    /// the charge within one wire pitch is equally weighted, i.e.
    /// no impact position dependency
    put(cfg, "wire_smear_ind", m_wire_smear_ind);
    put(cfg, "wire_smear_col", m_wire_smear_col);

    /// Transverse (wire) smearing -- re-distribute the charge onto
    /// nearby wires. A function of impact position, like field response.
    /// Attention: this is the residual field response (true / average).
    /// Normalization issue -- true, see percentage level charge bias
    /// for point source of charge at various locations within a wire.
    /// An interface, not used are present.
    put(cfg, "smear_response", m_smear_response_tn);

    /// gain for truth -- sign of charge in the output
    put(cfg, "truth_gain", m_truth_gain);

    /// Allow for a custom starting frame number
    put(cfg, "first_frame_number", m_frame_count);

    /// Name of component providing the anode plane.
    put(cfg, "anode", m_anode_tn);
    put(cfg, "rng", m_rng_tn);

    return cfg;
}

void Gen::TruthSmearer::configure(const WireCell::Configuration& cfg)
{
    m_anode_tn = get<string>(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);
    if (!m_anode) {
        cerr << "TruthSmearer[" << (void*) this << "]: failed to get anode: \"" << m_anode_tn << "\"\n";
        return;  // fixme: should throw something!
    }

    m_nsigma = get<double>(cfg, "nsigma", m_nsigma);
    m_continuous = get<bool>(cfg, "continuous", m_continuous);
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
    m_time_smear = get<double>(cfg, "time_smear", m_time_smear);
    m_wire_smear_ind = get<double>(cfg, "wire_smear_ind", m_wire_smear_ind);
    m_wire_smear_col = get<double>(cfg, "wire_smear_col", m_wire_smear_col);

    /// An interface, not used. Exact "Truth" after signal processing.
    m_smear_response_tn = get<string>(cfg, "smear_response", m_smear_response_tn);

    m_truth_gain = get<double>(cfg, "truth_gain", m_truth_gain);

    m_frame_count = get<int>(cfg, "first_frame_number", m_frame_count);
}

void Gen::TruthSmearer::process(output_queue& frames)
{
    ITrace::vector traces;
    double tick = -1;

    for (auto face : m_anode->faces()) {
        // Select the depos which are in this face's sensitive volume
        IDepo::vector face_depos;
        auto bb = face->sensitive();
        for (auto depo : m_depos) {
            if (bb.inside(depo->pos())) {
                face_depos.push_back(depo);
            }
        }

        {  // debugging
            auto ray = bb.bounds();
            cerr << "TruthSmearer: anode:" << m_anode->ident() << " face:" << face->ident() << ": processing "
                 << face_depos.size() << " depos "
                 << "with bb: " << ray.first << " --> " << ray.second << "\n";
        }

        for (auto plane : face->planes()) {
            const Pimpos* pimpos = plane->pimpos();

            Binning tbins(m_readout_time / m_tick, m_start_time, m_start_time + m_readout_time);

            if (tick < 0) {  // fixme: assume same tick for all.
                tick = tbins.binsize();
            }

            Gen::BinnedDiffusion bindiff(*pimpos, tbins, m_nsigma, m_rng);
            for (auto depo : face_depos) {
                // time filter smearing
                double extent_time = depo->extent_long() / m_drift_speed;
                bindiff.add(depo, sqrt(extent_time * extent_time + m_time_smear * m_time_smear), depo->extent_tran());
            }

            auto& wires = plane->wires();
            int planeid = plane->ident();

            double wire_smear = 1.0;
            if (planeid == 2) {
                wire_smear = m_wire_smear_col;
            }
            else if (planeid == 0 || planeid == 1) {
                wire_smear = m_wire_smear_ind;
            }
            else {
                std::cerr << "Truthsmearer: planeid " << planeid << " cannot be identified!" << std::endl;
            }

            auto ib = pimpos->impact_binning();
            auto rb = pimpos->region_binning();

            const double pitch = rb.binsize();
            const double impact = ib.binsize();
            const int nwires = rb.nbins();
            for (int iwire = 0; iwire < nwires; ++iwire) {
                ///  Similar to ImpactZipper::waveform
                ///  No convolution
                ///  m_waveform from BinnedDiffusion::impact_data()

                const double wire_pos = rb.center(iwire);

                // impact positions within +/-1 wires
                const int min_impact = ib.edge_index(wire_pos - 1.5 * pitch + 0.1 * impact);
                const int max_impact = ib.edge_index(wire_pos + 1.5 * pitch - 0.1 * impact);
                const int nsamples = bindiff.tbins().nbins();

                // total waveform for iwire
                Waveform::realseq_t total_spectrum(nsamples, 0.0);

                // ATTENTION: the bin center of max_imapct is in +2 wire pitch
                for (int imp = min_impact; imp < max_impact; imp++) {
                    // charge weight
                    double wire_weight = 1.0;

                    /// it is doable to read in a JsonArray of the wire smear
                    /// and use relative bin to target wire as array index
                    /// fo each wire (or impact position) smear
                    const double rel_imp_pos = ib.center(imp) - wire_pos;
                    const double dist_to_wire = abs(rel_imp_pos / rb.binsize());
                    if (dist_to_wire < 0.5) {
                        wire_weight = wire_smear * 1.0;
                    }
                    else if (dist_to_wire > 0.5 && dist_to_wire < 1.5) {
                        wire_weight = 0.5 * (1.0 - wire_smear);
                    }
                    else {
                        // should not happen
                        wire_weight = 0.0;
                        std::cerr << "TruthSmearer: impact " << imp << " position: " << ib.center(imp)
                                  << " out of +/-1 wire region or at wire boundary,  wire pitch: " << rb.binsize()
                                  << ", " << pitch << ", target wire position: " << wire_pos << std::endl;
                    }

                    // ImpactData
                    auto id = bindiff.impact_data(imp);
                    if (!id) {
                        continue;
                    }

                    Waveform::realseq_t charge_spectrum = id->waveform();
                    if (charge_spectrum.empty()) {
                        std::cerr << "impactZipper: no charge spectrum for absolute impact number: " << imp << endl;
                        continue;
                    }

                    // debugging
                    /* std::cout<<"TruthSmearer: planeid: " << planeid << " relative impact position: "<< rel_imp_pos */
                    /*     << " charge weight: "<<wire_weight*m_truth_gain<<endl; */
                    /* auto xx = Waveform::edge(charge_spectrum); */
                    /* //debugging */
                    /* std::cout<<"TruthSmearer: wire: "<< iwire << " impact: " << imp << " charge spectrum edges: "<<
                     * xx.first << ", " << xx.second << std::endl; */
                    Waveform::scale(charge_spectrum, wire_weight * m_truth_gain);
                    Waveform::increase(total_spectrum, charge_spectrum);
                }

                bindiff.erase(0, min_impact);

                /// end: charge wire smearing per wire filter

                auto mm = Waveform::edge(total_spectrum);
                if (mm.first == (int) total_spectrum.size()) {  // all zero
                    // std::cout<<"TruthSmearer: all zero wave spectrum at wire: "<< iwire << std::endl;
                    continue;
                }

                int chid = wires[iwire]->channel();
                int tbin = mm.first;
                ITrace::ChargeSequence charge(total_spectrum.begin() + mm.first, total_spectrum.begin() + mm.second);
                auto trace = make_shared<SimpleTrace>(chid, tbin, charge);
                traces.push_back(trace);
            }
        }
    }

    auto frame = make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, tick);
    frames.push_back(frame);

    // fixme: what about frame overflow here?  If the depos extend
    // beyond the readout where does their info go?  2nd order,
    // diffusion and finite field response can cause depos near the
    // end of the readout to have some portion of their waveforms
    // lost?
    m_depos.clear();

    m_start_time += m_readout_time;
    ++m_frame_count;
}

// Return true if ready to start processing and capture start time if
// in continuous mode.
bool Gen::TruthSmearer::start_processing(const input_pointer& depo)
{
    if (!depo) {
        return true;
    }

    if (!m_continuous) {
        if (m_depos.empty()) {
            m_start_time = depo->time();
            return false;
        }
    }

    // Note: we use this depo time even if it may not actually be
    // inside our sensitive volume.
    bool ok = depo->time() > m_start_time + m_readout_time;
    return ok;
}

bool Gen::TruthSmearer::operator()(const input_pointer& depo, output_queue& frames)
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
