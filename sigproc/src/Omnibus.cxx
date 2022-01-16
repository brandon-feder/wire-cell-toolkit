#include "WireCellSigProc/Omnibus.h"

#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/NamedFactory.h"
// #include "WireCellUtil/ExecMon.h"

#include "WireCellAux/FrameTools.h"

#include <iostream>

WIRECELL_FACTORY(Omnibus, WireCell::SigProc::Omnibus, WireCell::IApplication, WireCell::IConfigurable)

using namespace WireCell;

SigProc::Omnibus::Omnibus() {}

SigProc::Omnibus::~Omnibus() {}

WireCell::Configuration SigProc::Omnibus::default_configuration() const
{
    Configuration cfg;
    cfg["source"] = "";  // required
    cfg["filters"] = Json::arrayValue;
    cfg["sink"] = "";  // optional
    return cfg;
}

void SigProc::Omnibus::configure(const WireCell::Configuration& cfg)
{
    m_input_tn = cfg["source"].asString();  // keep for printouts later
    m_input = Factory::find_tn<IFrameSource>(m_input_tn);

    m_output_tn = cfg["sink"].asString();  // keep for printouts later
    if (m_output_tn.empty()) {
        std::cerr << "Omnibus has no final frame sink component.\n";
        m_output = nullptr;
    }
    else {
        m_output = Factory::find_tn<IFrameSink>(m_output_tn);
    }

    m_filters.clear();
    auto jffl = cfg["filters"];
    for (auto jff : jffl) {
        std::string fftn = jff.asString();
        std::cerr << "Omnibus adding frame filter: \"" << fftn << "\"\n";
        auto ff = Factory::find_tn<IFrameFilter>(fftn);
        m_filters.push_back(ff);
        m_filters_tn.push_back(fftn);
    }
}

void SigProc::Omnibus::execute()
{
    // ExecMon em("omnibus starts");

    IFrame::pointer frame;
    if (!(*m_input)(frame)) {
        std::cerr << "Omnibus: failed to get input frame from " << m_input_tn << "\n";
        THROW(RuntimeError() << errmsg{"Omnibus: failed to get input frame"});
    }
    if (!frame) {
        std::cerr << "Omnibus: got null frame, forwarding, assuming we have reached EOS\n";
    }
    else {
        if (!frame->traces()->size()) {
            std::cerr << "Omnibus: got empty input frame, something is busted\n";
            THROW(RuntimeError() << errmsg{"Omnibus: got empty input frame, something is busted"});
        }
        else {
            std::cerr << "Omnibus: got input frame from " << m_input_tn << " with " << frame->traces()->size()
                      << " traces\n";
            Aux::dump_frame(frame);
        }
    }

    // em("sourced frame");

    int count = 0;
    for (auto ff : m_filters) {
        std::string tn = m_filters_tn[count++];
        IFrame::pointer nextframe;
        if (!(*ff)(frame, nextframe)) {
            std::cerr << "Failed to filter frame from " << tn << "\n";  // fixme, give more info
            THROW(RuntimeError() << errmsg{"failed to filter frame"});
        }
        if (!nextframe && !frame) {
            continue;  // processing EOS
        }
        if (!nextframe) {
            std::cerr << "Omnibus: filter " << tn << " returned a null frame\n";
            THROW(RuntimeError() << errmsg{"filter returned a null frame"});
        }
        // em("filtered frame from " + tn);

        frame = nextframe;
        nextframe = nullptr;
        // em("dropped input to " + tn);

        if (frame) {
            std::cerr << "Omnibus: got frame from " << tn << " with " << frame->traces()->size() << " traces\n";
            Aux::dump_frame(frame);
        }
    }

    if (m_output) {
        if (!(*m_output)(frame)) {
            std::cerr << "Omnibus: failed to send output frame to " << m_output_tn << "\n";
            THROW(RuntimeError() << errmsg{"failed to send output frame"});
        }
    }
    // em("sunk frame");

    frame = nullptr;
    // em("dropped output frame");

    // std::cerr << em.summary() << std::endl;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
