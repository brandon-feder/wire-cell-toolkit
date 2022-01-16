#include "WireCellGen/ImpactZipper.h"
#include "WireCellUtil/Testing.h"

#include <iostream>  // debugging.
using namespace std;

using namespace WireCell;
Gen::ImpactZipper::ImpactZipper(IPlaneImpactResponse::pointer pir, BinnedDiffusion& bd)
  : m_pir(pir)
  , m_bd(bd)
{
}

Gen::ImpactZipper::~ImpactZipper() {}

Waveform::realseq_t Gen::ImpactZipper::waveform(int iwire) const
{
    const double pitch_range = m_pir->pitch_range();

    const auto pimpos = m_bd.pimpos();
    const auto rb = pimpos.region_binning();
    const auto ib = pimpos.impact_binning();
    const double wire_pos = rb.center(iwire);

    const int min_impact = ib.edge_index(wire_pos - 0.5 * pitch_range);
    const int max_impact = ib.edge_index(wire_pos + 0.5 * pitch_range);
    const int nsamples = m_bd.tbins().nbins();
    Waveform::compseq_t total_spectrum(nsamples, Waveform::complex_t(0.0, 0.0));

    int nfound = 0;
    const bool share = true;
    // const Waveform::complex_t complex_one_half(0.5,0.0);

    // The BinnedDiffusion is indexed by absolute impact and the
    // PlaneImpactResponse relative impact.
    for (int imp = min_impact; imp <= max_impact; ++imp) {
        // ImpactData
        auto id = m_bd.impact_data(imp);
        if (!id) {
            // common as we are scanning all impacts covering a wire
            // fixme: is there a way to predict this to avoid the query?
            // std::cerr << "ImpactZipper: no data for absolute impact number: " << imp << std::endl;
            continue;
        }

        const Waveform::compseq_t& charge_spectrum = id->spectrum();
        // for interpolation
        const Waveform::compseq_t& weightcharge_spectrum = id->weight_spectrum();

        if (charge_spectrum.empty()) {
            // should not happen
            std::cerr << "ImpactZipper: no charge for absolute impact number: " << imp << std::endl;
            continue;
        }
        if (weightcharge_spectrum.empty()) {
            // weight == 0, should not happen
            std::cerr << "ImpactZipper: no weight charge for absolute impact number: " << imp << std::endl;
            continue;
        }

        const double imp_pos = ib.center(imp);
        const double rel_imp_pos = imp_pos - wire_pos;
        // std::cerr << "IZ: " << " imp=" << imp << " imp_pos=" << imp_pos << " rel_imp_pos=" << rel_imp_pos <<
        // std::endl;

        Waveform::compseq_t conv_spectrum(nsamples, Waveform::complex_t(0.0, 0.0));
        if (share) {  // fixme: make a configurable option
            TwoImpactResponses two_ir = m_pir->bounded(rel_imp_pos);
            if (!two_ir.first || !two_ir.second) {
                // std::cerr << "ImpactZipper: no impact response for absolute impact number: " << imp << std::endl;
                continue;
            }
            // fixme: this is average, not interpolation.
            Waveform::compseq_t rs1 = two_ir.first->spectrum();
            Waveform::compseq_t rs2 = two_ir.second->spectrum();

            for (int ind = 0; ind < nsamples; ++ind) {
                // conv_spectrum[ind] = complex_one_half*(rs1[ind]+rs2[ind])*charge_spectrum[ind];

                // linear interpolation: wQ*rs1 + (Q-wQ)*rs2
                conv_spectrum[ind] = weightcharge_spectrum[ind] * rs1[ind] +
                                     (charge_spectrum[ind] - weightcharge_spectrum[ind]) * rs2[ind];
                /* debugging */
                /* if(iwire == 1000 && ind>1000 && ind<2000) { */
                /* std::cerr<<"rs1 spectrum: "<<imp<<"|"<<ind<<": "<<std::abs(rs1[ind])<<std::endl; */
                /* std::cerr<<"rs2 spectrum: "<<imp<<"|"<<ind<<": "<<std::abs(rs2[ind])<<std::endl; */
                /* std::cerr<<"rs1 charge spectrum "<<ind<<": "<<weightcharge_spectrum[ind]<<std::endl; */
                /* std::cerr<<"rs2 charge spectrum "<<ind<<":
                 * "<<charge_spectrum[ind]-weightcharge_spectrum[ind]<<std::endl; */
                /* //std::cerr<<"rs1 charge spectrum "<<ind<<": "<<complex_one_half*charge_spectrum[ind]<<std::endl; */
                /* //std::cerr<<"rs2 charge spectrum "<<ind<<": "<<complex_one_half*charge_spectrum[ind]<<std::endl; */
                /* } */
            }
        }
        else {
            auto ir = m_pir->closest(rel_imp_pos);
            if (!ir) {
                // std::cerr << "ImpactZipper: no impact response for absolute impact number: " << imp << std::endl;
                continue;
            }
            Waveform::compseq_t response_spectrum = ir->spectrum();
            for (int ind = 0; ind < nsamples; ++ind) {
                conv_spectrum[ind] = response_spectrum[ind] * charge_spectrum[ind];
            }
        }

        ++nfound;
        // std::cerr << "ImpactZipper: found:"<<nfound<<" for absolute impact number " << imp
        //           << " csize=" << csize << " rsize=" << rsize << " rebin=" << rebinfactor
        //           << std::endl;

        Waveform::increase(total_spectrum, conv_spectrum);
    }
    // std::cerr << "ImpactZipper: found " << nfound << " in abs impact: ["  << min_impact << ","<< max_impact << "]\n";

    // Clear memory assuming next call is iwire+1.
    // fixme: this is a dumb way to go. Better to make an iterator.
    m_bd.erase(0, min_impact);

    if (!nfound) {
        return Waveform::realseq_t(nsamples, 0.0);
    }

    auto waveform = Waveform::idft(total_spectrum);

    return waveform;
}
