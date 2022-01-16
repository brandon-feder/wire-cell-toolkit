#include "WireCellPytorch/TorchService.h"
#include "WireCellPytorch/Util.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/String.h"

WIRECELL_FACTORY(TorchService, 
                 WireCell::Pytorch::TorchService,
                 WireCell::ITensorForward,
                 WireCell::IConfigurable)

using namespace WireCell;



Pytorch::TorchService::TorchService()
    : Aux::Logger("TorchService", "torch")
{
}

Configuration Pytorch::TorchService::default_configuration() const
{
    Configuration cfg;

    // TorchScript model
    cfg["model"] = "model.ts";

    // one of: {cpu, gpu, gpuN} where "N" is a GPU number.  "gpu"
    // alone will use GPU 0.
    cfg["device"] = "cpu";
    
    return cfg;
}

void Pytorch::TorchService::configure(const WireCell::Configuration& cfg)
{
    auto dev = get<std::string>(cfg, "device", "cpu");
    m_ctx.connect(dev);

    auto model_path = cfg["model"].asString();
    if (model_path.empty()) {
        log->critical("no TorchScript model file provided");
        THROW(ValueError() << errmsg{"no TorchScript model file provided"});
    }

    // Use almost 1/2 the memory and 3/4 the time.
    torch::NoGradGuard no_grad;

    try {
        m_module = torch::jit::load(model_path, m_ctx.device());
    }
    catch (const c10::Error& e) {
        log->critical("error loading model: {} to {}: {}",
                      model_path, dev, e.what());
        throw;                  // rethrow
    }

    log->debug("loaded model {} to {}", model_path, dev);
}

ITensorSet::pointer Pytorch::TorchService::forward(const ITensorSet::pointer& in) const
{
    TorchSemaphore sem(m_ctx);

    log->debug("running model on {}", m_ctx.devname());

    torch::NoGradGuard no_grad;

    std::vector<torch::IValue> iival = Pytorch::from_itensor(in, m_ctx.is_gpu());

    torch::IValue oival;
    try {
        oival = m_module.forward(iival);
    }
    catch (const std::runtime_error& err) {
        log->error("error running model on {}: {}",
                   m_ctx.devname(), err.what());
        return nullptr;
    }

    ITensorSet::pointer ret = Pytorch::to_itensor({oival});

    return ret;
}
