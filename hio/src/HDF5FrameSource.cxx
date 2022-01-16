#include "WireCellHio/HDF5FrameSource.h"

#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/Array.h"

#include "WireCellHio/Util.h"

WIRECELL_FACTORY(HDF5FrameSource, WireCell::Hio::HDF5FrameSource, WireCell::IFrameSource, WireCell::IConfigurable)

using namespace WireCell;

Hio::HDF5FrameSource::HDF5FrameSource()
  : m_policy("")
  , l(Log::logger("hio"))
{
}

Hio::HDF5FrameSource::~HDF5FrameSource() {}

Configuration Hio::HDF5FrameSource::default_configuration() const
{
    Configuration cfg;
    cfg["anode"] = "AnodePlane";
    cfg["filelist"] = Json::arrayValue;  // list of input files, empties are skipped
    cfg["policy"] = "";                  // set to "stream" to avoid sending EOS after each file's worth of depos.

    // TODO: re-implement this with h5 meta data or some other ways
    cfg["sequence_max"] = 1;

    return cfg;
}

void Hio::HDF5FrameSource::configure(const WireCell::Configuration &cfg)
{
    m_cfg = cfg;

    m_anode = Factory::find_tn<IAnodePlane>(get<std::string>(cfg, "anode", ""));

    m_filenames = get<std::vector<std::string>>(cfg, "filelist");
    std::reverse(m_filenames.begin(), m_filenames.end());  // to use pop_back().

    m_policy = get<std::string>(cfg, "policy", "");
}

namespace {

    // used in sparsifying below.  Could use C++17 lambdas....
    bool ispositive(float x) { return x > 0.0; }
    bool isZero(float x) { return x == 0.0; }

    void eigen_to_traces(const Array::array_xxf &data, ITrace::vector &itraces, IFrame::trace_list_t &indices,
                         const int cbeg, const int nticks, const bool sparse = false)
    {
        // reuse this temporary vector to hold charge for a channel.
        ITrace::ChargeSequence charge(nticks, 0.0);

        for (int ch = cbeg; ch < cbeg + data.cols(); ++ch) {
            for (int itick = 0; itick != nticks; itick++) {
                const float q = data(itick, ch - cbeg);
                charge.at(itick) = q;
            }

            // actually save out
            if (sparse) {
                // Save waveform sparsely by finding contiguous, positive samples.
                std::vector<float>::const_iterator beg = charge.begin(), end = charge.end();
                auto i1 = std::find_if(beg, end, ispositive);  // first start
                while (i1 != end) {
                    // stop at next zero or end and make little temp vector
                    auto i2 = std::find_if(i1, end, isZero);
                    const std::vector<float> q(i1, i2);

                    // save out
                    const int tbin = i1 - beg;
                    SimpleTrace *trace = new SimpleTrace(ch, tbin, q);
                    const size_t trace_index = itraces.size();
                    indices.push_back(trace_index);
                    itraces.push_back(ITrace::pointer(trace));

                    // find start for next loop
                    i1 = std::find_if(i2, end, ispositive);
                }
            }
            else {
                // Save the waveform densely, including zeros.
                SimpleTrace *trace = new SimpleTrace(ch, 0, charge);
                const size_t trace_index = itraces.size();
                indices.push_back(trace_index);
                itraces.push_back(ITrace::pointer(trace));
            }
        }
    }
}  // namespace

bool Hio::HDF5FrameSource::operator()(IFrame::pointer &out)
{
    std::lock_guard<std::mutex> guard(g_h5cpp_mutex);
    l->trace("HDF5FrameSource {} : START", m_anode->ident());
    out = nullptr;

    const double start_time = -312500;  // ns
    const double tick = 500;            // ns

    const std::string tag = "orig";
    const int apa = m_anode->ident();

    if (m_frames.size() > 0) {
        out = m_frames.back();
        m_frames.pop_back();
        if (out)
            l->trace("HDF5FrameSource {} : pop {}", m_anode->ident(), out->ident());
        else
            l->trace("HDF5FrameSource {} : pop nullptr", m_anode->ident());
        return true;
    }

    while (!m_filenames.empty()) {
        auto fname = m_filenames.back();
        m_filenames.pop_back();
        h5::fd_t fd;
        try {
            fd = h5::open(fname, H5F_ACC_RDONLY);  // H5F_ACC_RDONLY
            l->trace("h5::open {}", fname);
        }
        catch (...) {
            l->error("Can't open {}", fname);
            break;
        }
        const int sequence_max = get<int>(m_cfg, "sequence_max", 0);
        for (int sequence = 0; sequence < sequence_max; ++sequence) {
            auto key = String::format("/%d/frame_%s%d", sequence, tag, apa);
            l->trace("loading: {}", key);

            Eigen::MatrixXf d;
            try {
                d = h5::read<Eigen::MatrixXf>(fd, key);
            }
            catch (...) {
                l->error("Can't load {}", key);
                break;
            }

            const int nticks = d.cols();
            l->trace("nticks = {}", nticks);

            ITrace::vector *itraces = new ITrace::vector;
            IFrame::trace_list_t trace_index;
            eigen_to_traces(d.transpose(), *itraces, trace_index, m_anode->channels().front(), nticks, false);

            SimpleFrame *sframe = new SimpleFrame(sequence, start_time, ITrace::shared_vector(itraces), tick);
            sframe->tag_frame("HDF5FrameSource");
            sframe->tag_traces(String::format("%s%d", tag, apa), trace_index);

            l->trace("produce {} traces: {}", itraces->size(), trace_index.size());

            m_frames.push_back(IFrame::pointer(sframe));
            if (m_policy != "stream") {
                m_frames.push_back(nullptr);
            }
        }
    }
    std::reverse(m_frames.begin(), m_frames.end());
    // std::cout << "[yuhw] : ";
    // for(auto fr : m_frames) {
    //   if(fr) std::cout << fr->ident() << " ";
    //   else std::cout << "null ";
    // }
    // std::cout << "\n";

    if (m_frames.empty()) {
        return false;
    }

    out = m_frames.back();
    m_frames.pop_back();
    if (out)
        l->trace("HDF5FrameSource {} : pop {}", m_anode->ident(), out->ident());
    else
        l->trace("HDF5FrameSource {} : pop nullptr", m_anode->ident());
    return true;
}