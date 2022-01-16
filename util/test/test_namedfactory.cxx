#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Logging.h"
#include "WireCellUtil/Exceptions.h"

using namespace WireCell;

class ISomeComponent : public virtual WireCell::IComponent<ISomeComponent> {
    Log::logptr_t l;

   public:
    ISomeComponent()
      : l(Log::logger("SomeComponent"))
    {
        l->debug("SomeComponent() at {:p}", (void*) this);
    }
    virtual ~ISomeComponent() { l->debug("~SomeComponent()"); }
    virtual void chirp() = 0;
};

class SomeConcrete : public virtual ISomeComponent {
    Log::logptr_t l;

   public:
    SomeConcrete()
      : l(Log::logger("SomeConcrete"))
    {
        l->debug("SomeConcrete() at {:p}", (void*) this);
    }
    virtual ~SomeConcrete() { l->debug("~SomeConcrete()"); }
    virtual void chirp() { l->info("SomeConcrete::chirp() at {:p}", (void*) this); }
};

WIRECELL_FACTORY(SomeConcrete, SomeConcrete, ISomeComponent)

using spdlog::debug;
using spdlog::info;
using spdlog::warn;

int main(int argc, char* argv[])
{
    Testing::log(argv[0]);
    // for this test we cheat since the factory isn't in a shared library.
    make_SomeConcrete_factory();

    auto ins = WireCell::Factory::lookup<ISomeComponent>("SomeConcrete");
    AssertMsg(ins, "Failed to lookup 'SomeConcrete' with interface 'ISomeComponent'");
    info("Got SomeConcrete @ {:p}", (void*) ins.get());
    info("Got bogus @ {:p}", (void*) std::shared_ptr<ISomeComponent>().get());
    ins->chirp();

    bool caught = false;
    try {
        auto should_fail = WireCell::Factory::lookup<ISomeComponent>("NothingNamedThis");
    }
    catch (WireCell::FactoryException& e) {
        warn(errstr(e));
        info("^^^ Successfully failed to lookup a nonexistent component");
        caught = true;
    }
    AssertMsg(caught, "Failed to throw");
    info("Should not hear anything from this failed find_tn");
    std::shared_ptr<ISomeComponent> ptr = WireCell::Factory::find_maybe_tn<ISomeComponent>("SomeConcrete:doesnotexist");
    AssertMsg(ptr == nullptr, "Got non null for nonexistent named component");
    info("Success.  Next, 'find' should be successful");
    std::shared_ptr<ISomeComponent> ptr2 = WireCell::Factory::find_maybe_tn<ISomeComponent>("SomeConcrete");
    debug("Got ptr2 @ {:p}", (void*) ptr2.get());
    AssertMsg(ptr2 != nullptr, "Got null for existing component");
    //    AssertMsg(false, "Got null for existing component");
}
