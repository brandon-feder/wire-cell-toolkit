#include "WireCellGen/DepoFramer.h"
#include "WireCellAux/FrameTools.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(DepoFramer, WireCell::Gen::DepoFramer, WireCell::IDepoFramer, WireCell::IConfigurable)

using namespace WireCell;

Gen::DepoFramer::DepoFramer(const std::string& drifter, const std::string& ductor)
  : m_drifter_tn(drifter)
  , m_ductor_tn(ductor)
{
}

Gen::DepoFramer::~DepoFramer() {}

WireCell::Configuration Gen::DepoFramer::default_configuration() const
{
    Configuration cfg;
    put(cfg, "Drifter", m_drifter_tn);
    put(cfg, "Ductor", m_ductor_tn);
    return cfg;
}

void Gen::DepoFramer::configure(const WireCell::Configuration& cfg)
{
    m_drifter = Factory::find_tn<IDrifter>(get(cfg, "Drifter", m_drifter_tn));
    m_ductor = Factory::find_tn<IDuctor>(get(cfg, "Ductor", m_ductor_tn));
}

bool Gen::DepoFramer::operator()(const input_pointer& in, output_pointer& out)
{
    const int ident = in->ident();

    // get depos into a mutable vector, sort and terminate
    auto sdepos = in->depos();
    std::vector<IDepo::pointer> depos(sdepos->begin(), sdepos->end()), drifted;
    std::sort(depos.begin(), depos.end(), descending_time);
    depos.push_back(nullptr);

    m_drifter->reset();
    for (auto depo : depos) {
        IDrifter::output_queue dq;
        (*m_drifter)(depo, dq);
        for (auto d : dq) {
            drifted.push_back(d);
        }
    }
    if (drifted.back()) {
        // check if drifter is following protocol
        std::cerr << "Gen::DepoFramer: warning: failed to get null on last drifted depo\n";
        drifted.push_back(nullptr);
    }

    m_ductor->reset();

    std::vector<IFrame::pointer> partial_frames;

    for (auto drifted_depo : drifted) {
        IDuctor::output_queue frames;
        (*m_ductor)(drifted_depo, frames);
        for (auto f : frames) {
            partial_frames.push_back(f);
        }
    }

    out = Aux::sum(partial_frames, ident);

    return true;
}
