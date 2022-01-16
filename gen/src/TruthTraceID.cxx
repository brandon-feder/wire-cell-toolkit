#include "WireCellGen/TruthTraceID.h"
#include "WireCellGen/BinnedDiffusion.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/Point.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/SimpleTrace.h"
#include "WireCellIface/SimpleFrame.h"

#include <string>

WIRECELL_FACTORY(TruthTraceID, WireCell::Gen::TruthTraceID, WireCell::IDuctor, WireCell::IConfigurable)

using namespace std;
using namespace WireCell;

Gen::TruthTraceID::TruthTraceID()
  : m_anode_tn("AnodePlane")
  , m_rng_tn("Random")
  , m_start_time(0.0 * units::ns)
  , m_readout_time(5.0 * units::ms)
  , m_tick(0.5 * units::us)
  , m_pitch_range(20 * 3 * units::mm)  // +/- 10 wires
  , m_drift_speed(1.0 * units::mm / units::us)
  , m_nsigma(3.0)
  , m_truth_gain(-1.0)
  , m_fluctuate(true)
  , m_frame_count(0)
  , m_eos(false)
  , m_truth_type("Bare")
  , m_num_ind_wire(2400.0)
  , m_num_col_wire(3456.0)
  , m_ind_sigma(1. / std::sqrt(3.1415926) * 1.4)
  , m_col_sigma(1. / std::sqrt(3.1415926) * 3.0)
  , m_time_sigma(0.14 * units::megahertz)
  , m_wire_power(2.0)
  , m_time_power(2.0)
  , m_max_wire_freq(1.0)
  , m_max_time_freq(1.0 * units::megahertz)
  , m_wire_flag(false)
  , m_time_flag(true)
{
}

WireCell::Configuration Gen::TruthTraceID::default_configuration() const
{
    Configuration cfg;
    put(cfg, "nsigma", m_nsigma);
    put(cfg, "fluctuate", m_fluctuate);
    put(cfg, "start_time", m_start_time);
    put(cfg, "readout_time", m_readout_time);
    put(cfg, "tick", m_tick);
    put(cfg, "pitch_range", m_pitch_range);
    put(cfg, "drift_speed", m_drift_speed);
    put(cfg, "first_frame_number", m_frame_count);
    put(cfg, "anode", m_anode_tn);
    put(cfg, "rng", m_rng_tn);
    put(cfg, "truth_type", m_truth_type);
    put(cfg, "number_induction_wire", m_num_ind_wire);
    put(cfg, "number_collection_wire", m_num_col_wire);
    put(cfg, "induction_sigma", m_ind_sigma);
    put(cfg, "collection_sigma", m_col_sigma);
    put(cfg, "time_sigma", m_time_sigma);
    put(cfg, "wire_power", m_wire_power);
    put(cfg, "time_power", m_time_power);
    put(cfg, "max_wire_frequency", m_max_wire_freq);
    put(cfg, "max_time_frequency", m_max_time_freq);
    put(cfg, "wire_filter_flag", m_wire_flag);
    put(cfg, "time_filter_flag", m_time_flag);
    put(cfg, "truth_gain", m_truth_gain);
    return cfg;
}

void Gen::TruthTraceID::configure(const WireCell::Configuration& cfg)
{
    m_anode_tn = get<string>(cfg, "anode", m_anode_tn);
    m_anode = Factory::find_tn<IAnodePlane>(m_anode_tn);
    if (!m_anode) {
        cerr << "Gen::Truth: failed to get anode: \"" << m_anode_tn << "\"\n";
        return;
    }

    m_nsigma = get<double>(cfg, "nsigma", m_nsigma);
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
    m_truth_gain = get<double>(cfg, "truth_gain", m_truth_gain);

    m_truth_type = get<string>(cfg, "truth_type", m_truth_type);
    m_num_ind_wire = get<double>(cfg, "number_induction_wire", m_num_ind_wire);
    m_num_col_wire = get<double>(cfg, "number_collection_wire", m_num_col_wire);
    m_ind_sigma = get<double>(cfg, "induction_sigma", m_ind_sigma);
    m_col_sigma = get<double>(cfg, "collection_sigma", m_col_sigma);
    m_time_sigma = get<double>(cfg, "time_sigma", m_time_sigma);
    m_wire_power = get<double>(cfg, "wire_power", m_wire_power);
    m_time_power = get<double>(cfg, "time_power", m_time_power);
    m_max_wire_freq = get<double>(cfg, "max_wire_frequency", m_max_wire_freq);
    m_max_time_freq = get<double>(cfg, "max_time_frequency", m_max_time_freq);
    m_wire_flag = get<bool>(cfg, "wire_filter_flag", m_wire_flag);
    m_time_flag = get<bool>(cfg, "time_filter_flag", m_time_flag);
}

void Gen::TruthTraceID::process(output_queue& frames)
{
    ITrace::vector traces;
    double tick = -1;

    // ### construct wire filters ###
    Binning indBins(m_num_ind_wire, 0.0, m_max_wire_freq);
    Response::HfFilter hf_ind(m_ind_sigma, m_wire_power, m_wire_flag);
    auto indTruth = hf_ind.generate(indBins);

    Binning colBins(m_num_col_wire, 0.0, m_max_wire_freq);
    Response::HfFilter hf_col(m_col_sigma, m_wire_power, m_wire_flag);
    auto colTruth = hf_col.generate(colBins);

    const int nbins = m_readout_time / m_tick;

    for (auto face : m_anode->faces()) {
        for (auto plane : face->planes()) {
            const Pimpos* pimpos = plane->pimpos();

            Binning tbins(nbins, m_start_time, m_start_time + m_readout_time);
            if (tick < 0) {
                tick = tbins.binsize();
            }

            // ### construct time filter ###
            Binning timeBins(tbins.nbins(), 0.0, m_max_time_freq);
            Response::HfFilter hf_time(m_time_sigma, m_time_power, m_time_flag);
            auto timeTruth = hf_time.generate(timeBins);

            // ### apply diffusion at wire plane ###
            Gen::BinnedDiffusion bindiff(*pimpos, tbins, m_nsigma, m_rng);
            for (auto depo : m_depos) {
                bindiff.add(depo, depo->extent_long() / m_drift_speed, depo->extent_tran());

                auto& wires = plane->wires();
                const int numwires = pimpos->region_binning().nbins();
                for (int iwire = 0; iwire < numwires; iwire++) {
                    const auto rbins = pimpos->region_binning();
                    const auto ibins = pimpos->impact_binning();
                    const double wire_pos = rbins.center(iwire);
                    // fixme (?) this under-calculates the min/max impact position by 1/2 wire region (bv).
                    const int min_impact = ibins.edge_index(wire_pos - 0.5 * m_pitch_range);
                    const int max_impact = ibins.edge_index(wire_pos + 0.5 * m_pitch_range);
                    const int nsamples = tbins.nbins();
                    Waveform::compseq_t total_spectrum(nsamples, Waveform::complex_t(0.0, 0.0));

                    for (int imp = min_impact; imp <= max_impact; imp++) {
                        auto id = bindiff.impact_data(imp);
                        if (!id) {
                            continue;
                        }

                        // debugging
                        std::cout << "Truth: charge spectrum extracted." << imp << std::endl;

                        if (m_truth_type == "Bare") {
                            const Waveform::compseq_t& charge_spectrum = id->spectrum();
                            if (charge_spectrum.empty()) {
                                continue;
                            }
                            Waveform::increase(total_spectrum, charge_spectrum);
                        }
                        else {  // ### convolve with charge w/ hf filters ###
                            const Waveform::compseq_t& charge_spectrum = id->spectrum();
                            if (charge_spectrum.empty()) {
                                continue;
                            }
                            Waveform::compseq_t conv_spectrum(nsamples, Waveform::complex_t(0.0, 0.0));
                            std::complex<float> charge_gain = m_truth_gain;
                            for (int ind = 0; ind < nsamples; ind++) {
                                if (wires[iwire]->channel() < 4800) {
                                    conv_spectrum[ind] =
                                        charge_gain * charge_spectrum[ind] * timeTruth[ind] * indTruth[iwire];
                                }
                                else {
                                    conv_spectrum[ind] =
                                        charge_gain * charge_spectrum[ind] * timeTruth[ind] * colTruth[iwire];
                                }
                            }
                            Waveform::increase(total_spectrum, conv_spectrum);
                        }
                    }
                    bindiff.erase(0, min_impact);

                    Waveform::realseq_t wave(nsamples, 0.0);
                    wave = Waveform::idft(total_spectrum);
                    auto mm = Waveform::edge(wave);
                    if (mm.first == (int) wave.size()) {
                        continue;
                    }

                    // ### push to trace ###
                    int chid = wires[iwire]->channel();
                    ITrace::ChargeSequence charge(wave.begin() + mm.first, wave.begin() + mm.second);
                    auto trace = make_shared<SimpleTrace>(chid, mm.first, charge);
                    traces.push_back(trace);
                }
            }
        }
    }

    // ### push to frame ###
    auto frame = make_shared<SimpleFrame>(m_frame_count, m_start_time, traces, tick);
    frames.push_back(frame);

    m_depos.clear();
    m_start_time += m_readout_time;
    ++m_frame_count;
}

void Gen::TruthTraceID::reset()
{
    m_depos.clear();
    m_eos = false;
}

bool Gen::TruthTraceID::operator()(const input_pointer& depo, output_queue& frames)
{
    if (m_eos) {
        return false;
    }

    double target_time = m_start_time + m_readout_time;
    if (!depo || depo->time() > target_time) {
        process(frames);
    }

    if (depo) {
        m_depos.push_back(depo);
    }
    else {
        m_eos = true;
    }

    return true;
}
