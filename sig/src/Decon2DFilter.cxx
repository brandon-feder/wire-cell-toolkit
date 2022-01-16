#include "WireCellSig/Decon2DFilter.h"
#include "WireCellSig/Util.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Array.h"
#include "WireCellUtil/Response.h"
#include "WireCellUtil/FFTBestLength.h"
#include "WireCellUtil/Exceptions.h"

#include "WireCellIface/ITensorSet.h"
#include "WireCellIface/IFilterWaveform.h"

#include "WireCellAux/SimpleTensorSet.h"
#include "WireCellAux/SimpleTensor.h"
#include "WireCellAux/Util.h"
#include "WireCellAux/TensUtil.h"

WIRECELL_FACTORY(Decon2DFilter, WireCell::Sig::Decon2DFilter, WireCell::ITensorSetFilter, WireCell::IConfigurable)

using namespace WireCell;

Sig::Decon2DFilter::Decon2DFilter()
  : log(Log::logger("sig"))
{
}

Configuration Sig::Decon2DFilter::default_configuration() const
{
    Configuration cfg;

    return cfg;
}

void Sig::Decon2DFilter::configure(const WireCell::Configuration &cfg) { m_cfg = cfg; }

bool Sig::Decon2DFilter::operator()(const ITensorSet::pointer &in, ITensorSet::pointer &out)
{
    log->debug("Decon2DFilter: start");

    if (!in) {
        out = nullptr;
        return true;
    }
    auto set_md = in->metadata();

    auto tag = get<std::string>(m_cfg, "tag", "trace_tag");

    auto iwf_ten = Aux::get_tens(in, tag, "waveform");
    if (!iwf_ten) {
        log->error("Tensor->Frame: failed to get waveform tensor for tag {}", tag);
        THROW(ValueError() << errmsg{"Sig::Decon2DFilter::operator() !iwf_ten"});
    }
    auto wf_shape = iwf_ten->shape();
    size_t nchans = wf_shape[0];
    size_t nticks = wf_shape[1];
    log->debug("iwf_ten->shape: {} {}", nchans, nticks);

    auto ch_ten = Aux::get_tens(in, tag, "channels");
    if (!ch_ten) {
        log->error("Tensor->Frame: failed to get channels tensor for tag {}", tag);
        THROW(ValueError() << errmsg{"Sig::Decon2DFilter::operator() !ch_ten"});
    }
    Eigen::Map<const Eigen::ArrayXi> ch_arr((const int *) ch_ten->data(), nchans);

    bool have_cmm = true;
    auto cmm_range = Aux::get_tens(in, tag, "bad:cmm_range");
    auto cmm_channel = Aux::get_tens(in, tag, "bad:cmm_channel");
    if (!cmm_range or !cmm_channel) {
        log->debug("Tensor->Frame: no optional cmm_range tensor for tag {}", tag);
        have_cmm = false;
    }

    int iplane = get<int>(m_cfg, "iplane", 0);
    log->debug("iplane {}", iplane);

    const std::vector<std::string> filter_names{"Wire_ind", "Wire_ind", "Wire_col"};

    // bins
    int m_pad_nwires = 0;

    // FIXME: figure this out
    int m_nwires = nchans;
    int m_nticks = nticks;

    // TensorSet to Eigen
    WireCell::Array::array_xxc c_data = Aux::itensor_to_eigen_array<std::complex<float>>(iwf_ten);
    log->debug("c_data: {} {}", c_data.rows(), c_data.cols());

    // make software filter on time
    WireCell::Waveform::realseq_t filter(c_data.cols(), 1.);
    for (auto icfg : m_cfg["filters"]) {
        const std::string filter_tn = icfg.asString();
        log->trace("filter_tn: {}", filter_tn);
        auto fw = Factory::find_tn<IFilterWaveform>(filter_tn);
        if (!fw) {
            THROW(ValueError() << errmsg{"!fw"});
        }
        auto wave = fw->filter_waveform(c_data.cols());
        for (size_t i = 0; i != wave.size(); i++) {
            filter.at(i) *= wave.at(i);
        }
    }

    // apply filter
    Array::array_xxc c_data_afterfilter(c_data.rows(), c_data.cols());
    for (int irow = 0; irow < c_data.rows(); ++irow) {
        for (int icol = 0; icol < c_data.cols(); ++icol) {
            c_data_afterfilter(irow, icol) = c_data(irow, icol) * filter.at(icol);
        }
    }

    // do the second round of inverse FFT on wire
    Array::array_xxf tm_r_data = Array::idft_cr(c_data_afterfilter, 0);
    Array::array_xxf r_data = tm_r_data.block(m_pad_nwires, 0, m_nwires, m_nticks);
    Sig::restore_baseline(r_data);

    // apply cmm
    if (have_cmm) {
        auto nranges = cmm_channel->shape()[0];
        assert(nranges == cmm_range->shape()[0]);
        assert(2 == cmm_range->shape()[1]);
        Eigen::Map<Eigen::ArrayXXd> ranges_arr((double *) cmm_range->data(), nranges, 2);
        Eigen::Map<Eigen::ArrayXd> channels_arr((double *) cmm_channel->data(), nranges);
        for (size_t ind = 0; ind < nranges; ++ind) {
            auto ch = channels_arr(ind);
            if (!(ch < r_data.rows())) {
                continue;
            }
            auto tmin = ranges_arr(ind, 0);
            auto tmax = ranges_arr(ind, 1);
            log->trace("ch: {}, tmin: {}, tmax: {}", ch, tmin, tmax);
            assert(tmin < tmax);
            assert(tmax <= r_data.cols());
            auto nbin = tmax - tmin;
            r_data.row(ch).segment(tmin, nbin) = 0.;
        }
    }

    // Eigen to TensorSet
    auto owf_ten = Aux::eigen_array_to_simple_tensor<float>(r_data);

    // save back to ITensorSet
    ITensor::vector *itv = new ITensor::vector;
    auto iwf_md = iwf_ten->metadata();
    auto &owf_md = owf_ten->metadata();
    owf_md["tag"] = tag;
    owf_md["pad"] = get<float>(iwf_md, "pad", 0.0);
    owf_md["tbin"] = get<float>(iwf_md, "tbin", 0.0);
    owf_md["type"] = "waveform";
    itv->push_back(ITensor::pointer(owf_ten));

    // FIXME need to change ch_ten tag to outtag
    itv->push_back(ch_ten);

    Configuration oset_md(in->metadata());
    oset_md["tags"] = Json::arrayValue;
    oset_md["tags"].append(tag);

    out = std::make_shared<Aux::SimpleTensorSet>(in->ident(), oset_md, ITensor::shared_vector(itv));

    log->debug("Decon2DFilter: end");

    return true;
}