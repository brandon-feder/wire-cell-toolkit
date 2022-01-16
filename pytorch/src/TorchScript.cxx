#include "WireCellPytorch/TorchScript.h"
#include "WireCellPytorch/Util.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/String.h"

WIRECELL_FACTORY(TorchScript,
                 WireCell::Pytorch::TorchScript,
                 WireCell::ITensorSetFilter,
                 WireCell::IConfigurable)

using namespace WireCell;

Pytorch::TorchScript::TorchScript()
    : Aux::Logger("TorchScript", "torch")
{
}

Configuration Pytorch::TorchScript::default_configuration() const
{
    Configuration cfg;

    // TorchScript model
    cfg["model"] = "model.ts";
    cfg["gpu"] = m_gpu;

    // if failed, wait this time and try again.
    // FIXME: This parameter breaks WCT units conventions!
    // It should be in WCT system of units or if it absolutely must break that then it should have a label hinting the units like "_ms".
    // To fix this now requires finding all usage in existing config and I don't know how prevalent that is.
    cfg["wait_time"] = m_wait_time/units::ms;

    // for debug
    // cfg["nloop"] = 10;

    return cfg;
}

void Pytorch::TorchScript::configure(const WireCell::Configuration& cfg)
{
    m_gpu = get<bool>(cfg, "gpu", false);

    // FIXME: we must do some double conversion due to the FIXME above.
    m_wait_time = get<double>(cfg, "wait_time", m_wait_time/units::ms)*units::ms;


    auto model_path = cfg["model"].asString();
    if (model_path.empty()) {
        log->critical("no TorchScript model file provided");
        THROW(ValueError() << errmsg{"no TorchScript model file provided"});
    }

    // load Torch Script Model
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        if (m_gpu) {
            m_module = torch::jit::load(model_path, at::kCUDA);
            m_module.to(at::kCUDA);
        }
        else {
            m_module = torch::jit::load(model_path);
            m_module.to(at::kCPU);
        }
        log->debug("Model: {} loaded on {}",
                   model_path, m_gpu ? "GPU" : "CPU");

    }
    catch (const c10::Error& e) {
        log->critical("error loading model: {}. {}", model_path, e.what());
        throw; // rethrow
    }
}

bool Pytorch::TorchScript::operator()(const ITensorSet::pointer& in, ITensorSet::pointer& out)
{
    if (!in) {
        out = nullptr;
        return true;
    }

    if (m_gpu) {
        m_module.to(at::kCUDA);
    }
    else {
        m_module.to(at::kCPU);
    }

    double thread_wait_time_ms = 0;

    // for(int iloop=0; iloop<m_cfg["nloop"].asInt();++iloop) {
    torch::IValue oival;
    while (true) {
        std::vector<torch::IValue> iival = Pytorch::from_itensor(in, m_gpu);
        try {
            oival = m_module.forward(iival);
            break;
        }
        catch (const std::runtime_error& e) {
            log->critical("error running model on {}: {}",
                          m_gpu ? "GPU" : "CPU", e.what());
            throw; // rethrow
        }
        catch (...) {
            int ms = m_wait_time / units::ms;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            thread_wait_time_ms += ms;
        }
    }
    // }

    out = Pytorch::to_itensor({oival});

    log->debug("thread_wait_time: {} sec", thread_wait_time_ms / 1000.);

    return true;
}
