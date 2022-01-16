#include "WireCellGen/ImpactTransform.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/FFTBestLength.h"
#include "WireCellUtil/Exceptions.h"
#include <iostream>  // debugging.
using namespace std;

using namespace WireCell;

Gen::ImpactTransform::ImpactTransform(IPlaneImpactResponse::pointer pir,
                                      BinnedDiffusion_transform& bd)
  : m_pir(pir)
  , m_bd(bd)
{
    // for (int i=0;i!=210;i++){
    //   double pos = -31.5 + 0.3*i+1e-9;0
    //   m_pir->closest(pos);
    // }

    // arrange the field response (210 in total, pitch_range/impact)
    // number of wires nwires ...
    m_num_group = std::round(m_pir->pitch() / m_pir->impact()) + 1;  // 11
    m_num_pad_wire = std::round((m_pir->nwires() - 1) / 2.);         // 10 for wires, 5 for PCB strips

    const auto pimpos = m_bd.pimpos();
    //  const int nsamples = m_bd.tbins().nbins();
    // const auto rb = pimpos.region_binning();
    // const int nwires = rb.nbins();

    //
    
    //std::cerr << "ImpactTransform: num_group:" << m_num_group << " num_pad_wire:" << m_num_pad_wire << std::endl;
    for (int i = 0; i != m_num_group; i++) {
        double rel_cen_imp_pos;
        if (i != m_num_group - 1) {
            rel_cen_imp_pos = -m_pir->pitch() / 2. + m_pir->impact() * i + 1e-9;
        }
        else {
            rel_cen_imp_pos = -m_pir->pitch() / 2. + m_pir->impact() * i - 1e-9;
        }
        m_vec_impact.push_back(std::round(rel_cen_imp_pos / m_pir->impact()));
        std::map<int, IImpactResponse::pointer> map_resp;  // already in freq domain

        //std::cerr << "ImpactTransform: " << rel_cen_imp_pos << std::endl;
        for (int j = 0; j != m_pir->nwires(); j++) {
            // std::cerr << "ImpactTransform: "
            //           << i << " " << j << " "
            //           << rel_cen_imp_pos - (j-m_num_pad_wire)*m_pir->pitch()<< " "
            //           << std::endl;

            try {
                map_resp[j - m_num_pad_wire] = m_pir->closest(rel_cen_imp_pos - (j - m_num_pad_wire) * m_pir->pitch());
            }
            catch (ValueError& ve) {
                std::cerr << "ImpactTransform: I angered PIR with: i="
                          << i << " j=" << j
                          << " rel_cen_imp_pos=" << rel_cen_imp_pos
                          << " m_num_pad_wire=" << m_num_pad_wire
                          << " m_num_group=" << m_num_group
                          << " pir->pitch=" << m_pir->pitch()
                          << " pir->impact=" << m_pir->impact()
                          << " pir->nwires=" << m_pir->nwires()
                          << " looking for: "
                          << rel_cen_imp_pos - (j - m_num_pad_wire) * m_pir->pitch()
                          << std::endl;                
                continue;
            }
                
            Waveform::compseq_t response_spectrum = map_resp[j - m_num_pad_wire]->spectrum();

            //response_spectrum.size() << std::endl;
        }
        // std::cout << m_vec_impact.back() << std::endl;
        // std::cout << rel_cen_imp_pos << std::endl;
        // std::cout << map_resp.size() << std::endl;
        m_vec_map_resp.push_back(map_resp);

        // Eigen::SparseMatrix<float> *mat = new Eigen::SparseMatrix<float>(nsamples,nwires);
        //  mat.reserve(Eigen::VectorXi::Constant(nwires,1000));
        // m_vec_spmatrix.push_back(mat);

        std::vector<std::tuple<int, int, double> > vec_charge;  // ch, time, charge
        m_vec_vec_charge.push_back(vec_charge);
    }

    // m_bd.get_charge_matrix(m_vec_spmatrix, m_vec_impact);
    // std::cout << nwires << " " << nsamples << std::endl;

    // now work on the charge part ...
    // trying to sampling ...
    m_bd.get_charge_vec(m_vec_vec_charge, m_vec_impact);
    // std::cout << nwires << " " << nsamples << std::endl;

    // for (size_t i=0;i!=m_vec_vec_charge.size();i++){
    //   std::cout << m_vec_vec_charge[i].size() << std::endl;
    // }

    // length and width ...

    //

    //    std::cout << nwires << " " << nsamples << std::endl;
    std::pair<int, int> impact_range = m_bd.impact_bin_range(m_bd.get_nsigma());
    std::pair<int, int> time_range = m_bd.time_bin_range(m_bd.get_nsigma());

    //  std::cout << impact_range.first << " " << impact_range.second << " " << time_range.first << " " <<
    //  time_range.second << std::endl;

    int start_ch = std::floor(impact_range.first * 1.0 / (m_num_group - 1)) - 1;
    int end_ch = std::ceil(impact_range.second * 1.0 / (m_num_group - 1)) + 2;
    if ((end_ch - start_ch) % 2 == 1) end_ch += 1;
    int start_tick = time_range.first - 1;
    int end_tick = time_range.second + 2;
    if ((end_tick - start_tick) % 2 == 1) end_tick += 1;

    //  m_decon_data = Array::array_xxf::Zero(nwires+2*m_num_pad_wire,nsamples);
    // for saving the accumulated wire data in the time frequency domain ...
    // adding no padding now, it make the FFT slower, need some other methods ...

    int npad_wire = 0;
    const size_t ntotal_wires = fft_best_length(end_ch - start_ch + 2 * m_num_pad_wire, 1);

    //   pow(2,std::ceil(log(end_ch - start_ch + 2 * m_num_pad_wire)/log(2)));
    //  if (nwires == 2400){
    // if (ntotal_wires > 2500)
    //   ntotal_wires = 2500;
    // }else if (nwires ==3456){
    // if (ntotal_wires > 3600)
    //   ntotal_wires = 3600;
    //      npad_wire=72; //3600
    //}
    npad_wire = (ntotal_wires - end_ch + start_ch) / 2;
    m_start_ch = start_ch - npad_wire;
    m_end_ch = end_ch + npad_wire;
    // std::cout << start_ch << " " << end_ch << " " << npad_wire << " " << start_tick << " " << end_tick << " " <<
    // m_start_ch << " " << m_end_ch << std::endl;

    int npad_time = m_pir->closest(0)->waveform_pad();
    const size_t ntotal_ticks = fft_best_length(end_tick - start_tick + npad_time);

    // pow(2,std::ceil(log(end_tick - start_tick + npad_time)/log(2)));
    // if (ntotal_ticks >9800 && nsamples <9800 && nsamples >9550)
    //  ntotal_ticks = 9800;
    npad_time = ntotal_ticks - end_tick + start_tick;
    m_start_tick = start_tick;
    m_end_tick = end_tick + npad_time;

    // m_end_tick = 16384;//nsamples;
    // m_start_tick = 0;
    // // std::cout << m_start_tick << " " << m_end_tick << std::endl;
    // int npad_time = 0;
    // int ntotal_ticks = pow(2,std::ceil(log(nsamples + npad_time)/log(2)));
    // if (ntotal_ticks >9800 && nsamples <9800)
    //   ntotal_ticks = 9800
    // npad_time = ntotal_ticks - nsamples;
    // m_start_tick = 0;
    // m_end_tick = ntotal_ticks;

    Array::array_xxc acc_data_f_w =
        Array::array_xxc::Zero(end_ch - start_ch + 2 * npad_wire, m_end_tick - m_start_tick);

    int num_double = (m_vec_vec_charge.size() - 1) / 2;
    // int num_double = (m_vec_spmatrix.size()-1)/2;

    // speed up version , first five
    for (int i = 0; i != num_double; i++) {
        // if (i!=0) continue;
        // std::cout << i << std::endl;
        Array::array_xxc c_data = Array::array_xxc::Zero(end_ch - start_ch + 2 * npad_wire, m_end_tick - m_start_tick);

        // fill normal order
        for (size_t j = 0; j != m_vec_vec_charge.at(i).size(); j++) {
            c_data(std::get<0>(m_vec_vec_charge.at(i).at(j)) + npad_wire - start_ch,
                   std::get<1>(m_vec_vec_charge.at(i).at(j)) - m_start_tick) +=
                std::get<2>(m_vec_vec_charge.at(i).at(j));
        }
        //    std::cout << i << " " << m_vec_vec_charge.at(i).size() << std::endl;
        m_vec_vec_charge.at(i).clear();
        m_vec_vec_charge.at(i).shrink_to_fit();

        // useing matrix form ...
        // for (int k=0; k<m_vec_spmatrix.at(i)->outerSize(); ++k)
        //   for (Eigen::SparseMatrix<float>::InnerIterator it(*m_vec_spmatrix.at(i),k); it; ++it){
        // 	c_data(it.col()+npad_wire-start_ch,it.row()-m_start_tick) = it.value();
        //   }
        // delete m_vec_spmatrix.at(i);
        // //m_vec_spmatrix.at(i).setZero();
        // //m_vec_spmatrix.at(i).resize(0,0);

        // fill reverse order
        int ii = num_double * 2 - i;
        for (size_t j = 0; j != m_vec_vec_charge.at(ii).size(); j++) {
            c_data(end_ch + npad_wire - 1 - std::get<0>(m_vec_vec_charge.at(ii).at(j)),
                   std::get<1>(m_vec_vec_charge.at(ii).at(j)) - m_start_tick) +=
                std::complex<float>(0, std::get<2>(m_vec_vec_charge.at(ii).at(j)));
        }
        //    std::cout << ii << " " << m_vec_vec_charge.at(ii).size() << std::endl;
        m_vec_vec_charge.at(ii).clear();
        m_vec_vec_charge.at(ii).shrink_to_fit();
        // for (int k=0; k<m_vec_spmatrix.at(ii)->outerSize(); ++k)
        //   for (Eigen::SparseMatrix<float>::InnerIterator it(*m_vec_spmatrix.at(ii),k); it; ++it){
        // 	c_data(it.col()+npad_wire-start_ch,it.row()-m_start_tick) = it.value();
        //   }
        // delete m_vec_spmatrix.at(ii);
        // //    m_vec_spmatrix.at(ii).setZero();
        // //m_vec_spmatrix.at(ii).resize(0,0);

        // Do FFT on time
        c_data = Array::dft_cc(c_data, 0);
        // Do FFT on wire
        c_data = Array::dft_cc(c_data, 1);

        // std::cout << i << std::endl;
        {
            Array::array_xxc resp_f_w =
                Array::array_xxc::Zero(end_ch - start_ch + 2 * npad_wire, m_end_tick - m_start_tick);
            {
                Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[0]->spectrum();
                // do a inverse FFT
                Waveform::realseq_t rs1_t = Waveform::idft(rs1);
                // pick the first xxx ticks
                Waveform::realseq_t rs1_reduced(m_end_tick - m_start_tick, 0);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs1_t.size())) break;
                    rs1_reduced.at(icol) = rs1_t[icol];
                }
                // do a FFT
                rs1 = Waveform::dft(rs1_reduced);

                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    resp_f_w(0, icol) = rs1[icol];
                }
            }

            for (int irow = 0; irow != m_num_pad_wire; irow++) {
                Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[irow + 1]->spectrum();
                Waveform::realseq_t rs1_t = Waveform::idft(rs1);
                Waveform::realseq_t rs1_reduced(m_end_tick - m_start_tick, 0);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs1_t.size())) break;
                    rs1_reduced.at(icol) = rs1_t[icol];
                }
                rs1 = Waveform::dft(rs1_reduced);
                Waveform::compseq_t rs2 = m_vec_map_resp.at(i)[-irow - 1]->spectrum();
                Waveform::realseq_t rs2_t = Waveform::idft(rs2);
                Waveform::realseq_t rs2_reduced(m_end_tick - m_start_tick, 0);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs2_t.size())) break;
                    rs2_reduced.at(icol) = rs2_t[icol];
                }
                rs2 = Waveform::dft(rs2_reduced);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    resp_f_w(irow + 1, icol) = rs1[icol];
                    resp_f_w(end_ch - start_ch - 1 - irow + 2 * npad_wire, icol) = rs2[icol];
                }
            }
            // std::cout << i << std::endl;

            // Do FFT on wire for response // slight larger
            resp_f_w = Array::dft_cc(resp_f_w, 1);  // Now becomes the f and f in both time and wire domain ...
            // multiply them together
            c_data = c_data * resp_f_w;
        }

        // Do inverse FFT on wire
        c_data = Array::idft_cc(c_data, 1);

        // Add to wire result in frequency
        acc_data_f_w += c_data;
    }

    // std::cout << "ABC : " << std::endl;

    // central region ...
    {
        int i = num_double;
        // fill response array in frequency domain

        Array::array_xxc data_f_w;
        {
            Array::array_xxf data_t_w =
                Array::array_xxf::Zero(end_ch - start_ch + 2 * npad_wire, m_end_tick - m_start_tick);
            // fill charge array in time-wire domain // slightly larger
            for (size_t j = 0; j != m_vec_vec_charge.at(i).size(); j++) {
                data_t_w(std::get<0>(m_vec_vec_charge.at(i).at(j)) + npad_wire - start_ch,
                         std::get<1>(m_vec_vec_charge.at(i).at(j)) - m_start_tick) +=
                    std::get<2>(m_vec_vec_charge.at(i).at(j));
                // std::cout << std::get<1>(m_vec_vec_charge.at(i).at(j)) << std::endl;
            }
            //      std::cout << i << " " << m_vec_vec_charge.at(i).size() << std::endl;
            m_vec_vec_charge.at(i).clear();
            m_vec_vec_charge.at(i).shrink_to_fit();
            // for (int k=0; k<m_vec_spmatrix.at(i)->outerSize(); ++k)
            // 	for (Eigen::SparseMatrix<float>::InnerIterator it(*m_vec_spmatrix.at(i),k); it; ++it){
            // 	  data_t_w(it.col()+npad_wire-start_ch,it.row()-m_start_tick) = it.value();
            // 	}
            // delete m_vec_spmatrix.at(i);
            // //      m_vec_spmatrix.at(i).setZero();
            // // m_vec_spmatrix.at(i).resize(0,0);

            // Do FFT on time
            data_f_w = Array::dft_rc(data_t_w, 0);
            // Do FFT on wire
            data_f_w = Array::dft_cc(data_f_w, 1);
        }

        {
            Array::array_xxc resp_f_w =
                Array::array_xxc::Zero(end_ch - start_ch + 2 * npad_wire, m_end_tick - m_start_tick);

            {
                Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[0]->spectrum();
                // Array::array_xxc temp_resp_f_w = Array::array_xxc::Zero(2*m_num_pad_wire+1,nsamples);
                // for (int icol = 0; icol != nsamples; icol++){
                //   temp_resp_f_w(0,icol) = rs1[icol];
                // }
                // Array::array_xxf temp_resp_t_w =
                // Array::idft_cr(temp_resp_f_w,0).block(0,0,2*m_num_pad_wire+1,m_end_tick-m_start_tick); temp_resp_f_w
                // = Array::dft_rc(temp_resp_t_w,0);

                // do a inverse FFT
                Waveform::realseq_t rs1_t = Waveform::idft(rs1);
                // pick the first xxx ticks
                Waveform::realseq_t rs1_reduced(m_end_tick - m_start_tick, 0);
                // std::cout << rs1.size() << " " << nsamples << " " << m_end_tick << " " <<  m_start_tick << std::endl;
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs1_t.size())) break;
                    rs1_reduced.at(icol) = rs1_t[icol];
                    //  std::cout << icol << " " << rs1_t[icol] << std::endl;
                }
                // do a FFT
                rs1 = Waveform::dft(rs1_reduced);

                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    //   std::cout << icol << " " << rs1[icol] << " " << temp_resp_f_w(0,icol) << std::endl;
                    resp_f_w(0, icol) = rs1[icol];
                }
            }
            for (int irow = 0; irow != m_num_pad_wire; irow++) {
                Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[irow + 1]->spectrum();
                Waveform::realseq_t rs1_t = Waveform::idft(rs1);
                Waveform::realseq_t rs1_reduced(m_end_tick - m_start_tick, 0);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs1_t.size())) break;
                    rs1_reduced.at(icol) = rs1_t[icol];
                }
                rs1 = Waveform::dft(rs1_reduced);
                Waveform::compseq_t rs2 = m_vec_map_resp.at(i)[-irow - 1]->spectrum();
                Waveform::realseq_t rs2_t = Waveform::idft(rs2);
                Waveform::realseq_t rs2_reduced(m_end_tick - m_start_tick, 0);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    if (icol >= int(rs2_t.size())) break;
                    rs2_reduced.at(icol) = rs2_t[icol];
                }
                rs2 = Waveform::dft(rs2_reduced);
                for (int icol = 0; icol != m_end_tick - m_start_tick; icol++) {
                    resp_f_w(irow + 1, icol) = rs1[icol];
                    resp_f_w(end_ch - start_ch - 1 - irow + 2 * npad_wire, icol) = rs2[icol];
                }
                // for (int icol = 0; icol != nsamples; icol++){
                //   resp_f_w(irow+1,icol) = rs1[icol];
                //   resp_f_w(end_ch-start_ch-1-irow+2*npad_wire,icol) = rs2[icol];
                // }
            }
            // Do FFT on wire for response // slight larger
            resp_f_w = Array::dft_cc(resp_f_w, 1);  // Now becomes the f and f in both time and wire domain ...
            // multiply them together
            data_f_w = data_f_w * resp_f_w;
        }

        // Do inverse FFT on wire
        data_f_w = Array::idft_cc(data_f_w, 1);

        // Add to wire result in frequency
        acc_data_f_w += data_f_w;
    }

    // m_decon_data = Array::array_xxc::Zero(nwires,nsamples);
    //    if (npad_wire!=0){
    acc_data_f_w = Array::idft_cc(acc_data_f_w, 0);  //.block(npad_wire,0,nwires,nsamples);
    Array::array_xxf real_m_decon_data = acc_data_f_w.real();
    Array::array_xxf img_m_decon_data = acc_data_f_w.imag().colwise().reverse();
    m_decon_data = real_m_decon_data + img_m_decon_data;

    // std::cout << real_m_decon_data(40,5182) << " " << img_m_decon_data(40,5182) << std::endl;
    //    std::cout << real_m_decon_data(40,5182-m_start_tick) << " " << img_m_decon_data(40,5182-m_start_tick) <<
    //    std::endl;

    //}else{
    // Array::array_xxc temp_m_decon_data = Array::idft_cc(acc_data_f_w,0);
    //   Array::array_xxf real_m_decon_data = temp_m_decon_data.real();
    //   Array::array_xxf img_m_decon_data = temp_m_decon_data.imag().rowwise().reverse();
    //   m_decon_data = real_m_decon_data + img_m_decon_data;
    // }

    // // prepare FFT, loop 11 of them ... (older version)
    // for (size_t i=0;i!=m_vec_vec_charge.size();i++){
    //   // fill response array in frequency domain
    //   if (i!=10) continue;

    //   Array::array_xxc data_f_w;
    //   {
    // 	Array::array_xxf data_t_w = Array::array_xxf::Zero(nwires+2*npad_wire,nsamples);
    // 	// fill charge array in time-wire domain // slightly larger
    // 	for (size_t j=0;j!=m_vec_vec_charge.at(i).size();j++){
    // 	  data_t_w(std::get<0>(m_vec_vec_charge.at(i).at(j))+npad_wire,std::get<1>(m_vec_vec_charge.at(i).at(j))) +=
    // std::get<2>(m_vec_vec_charge.at(i).at(j));
    // 	}
    // 	m_vec_vec_charge.at(i).clear();

    // 	// Do FFT on time
    // 	data_f_w = Array::dft_rc(data_t_w,0);
    // 	// Do FFT on wire
    // 	data_f_w = Array::dft_cc(data_f_w,1);
    //   }

    //   {
    // 	Array::array_xxc resp_f_w = Array::array_xxc::Zero(nwires+2*npad_wire,nsamples);
    // 	{
    // 	  Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[0]->spectrum();
    // 	  for (int icol = 0; icol != nsamples; icol++){
    // 	    resp_f_w(0,icol) = rs1[icol];
    // 	  }
    // 	}
    // 	for (int irow = 0; irow!=m_num_pad_wire;irow++){
    // 	  Waveform::compseq_t rs1 = m_vec_map_resp.at(i)[irow+1]->spectrum();
    // 	  Waveform::compseq_t rs2 = m_vec_map_resp.at(i)[-irow-1]->spectrum();
    // 	  for (int icol = 0; icol != nsamples; icol++){
    // 	    resp_f_w(irow+1,icol) = rs1[icol];
    // 	    resp_f_w(nwires-1-irow+2*npad_wire,icol) = rs2[icol];
    // 	  }
    // 	}
    // 	// Do FFT on wire for response // slight larger
    // 	resp_f_w = Array::dft_cc(resp_f_w,1); // Now becomes the f and f in both time and wire domain ...
    // 	// multiply them together
    // 	data_f_w = data_f_w * resp_f_w;
    //   }

    //   // Do inverse FFT on wire
    //   data_f_w = Array::idft_cc(data_f_w,1);

    //   // Add to wire result in frequency
    //   acc_data_f_w += data_f_w;
    // }
    // m_vec_vec_charge.clear();

    // // do inverse FFT on time for the final results ...

    // if (npad_wire!=0){
    //   Array::array_xxf temp_m_decon_data = Array::idft_cr(acc_data_f_w,0);
    //   m_decon_data = temp_m_decon_data.block(npad_wire,0,nwires,nsamples);
    // }else{
    //   m_decon_data = Array::idft_cr(acc_data_f_w,0);
    // }

    //    std::cout << m_decon_data(40,5195-m_start_tick)/units::mV << " " <<
    //    m_decon_data(40,5195-m_start_tick)/units::mV << std::endl;

    //  m_vec_spmatrix.clear();
    // m_vec_spmatrix.shrink_to_fit();

    // int nrows = resp_f_w.rows();
    // int ncols = resp_f_w.cols();
    // log->debug("ImpactTransform: # of channels: {} # of ticks: {}", m_decon_data.rows(), m_decon_data.cols());

}  // constructor

Gen::ImpactTransform::~ImpactTransform() {}

Waveform::realseq_t Gen::ImpactTransform::waveform(int iwire) const
{
    const int nsamples = m_bd.tbins().nbins();
    if (iwire < m_start_ch || iwire >= m_end_ch) {
        return Waveform::realseq_t(nsamples, 0.0);
    }
    else {
        Waveform::realseq_t wf(nsamples, 0.0);
        for (int i = 0; i != nsamples; i++) {
            if (i >= m_start_tick && i < m_end_tick) {
                wf.at(i) = m_decon_data(iwire - m_start_ch, i - m_start_tick);
            }
            else {
                // wf.at(i) = 1e-25;
            }
            // std::cout << m_decon_data(iwire-m_start_ch,i-m_start_tick) << std::endl;
        }

        if (m_pir->closest(0)->long_aux_waveform().size() > 0) {
            // now convolute with the long-range response ...
            const size_t nlength = fft_best_length(nsamples + m_pir->closest(0)->long_aux_waveform_pad());

            // nlength = nsamples;

            //   std::cout << nlength << " " << nsamples + m_pir->closest(0)->long_aux_waveform_pad() << std::endl;

            wf.resize(nlength, 0);
            Waveform::realseq_t long_resp = m_pir->closest(0)->long_aux_waveform();
            long_resp.resize(nlength, 0);
            Waveform::compseq_t spec = Waveform::dft(wf);
            Waveform::compseq_t long_spec = Waveform::dft(long_resp);
            for (size_t i = 0; i != nlength; i++) {
                spec.at(i) *= long_spec.at(i);
            }
            wf = Waveform::idft(spec);
            wf.resize(nsamples, 0);
        }

        return wf;
    }
}
