#include "WireCellUtil/Singleton.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Logging.h"

using spdlog::info;

class Foo {
   public:
    Foo() { info("Foo() at {:p}", (void*) this); }
    virtual ~Foo() { info("~Foo() at {:p}", (void*) this); }
    virtual void chirp() { info("Foo::chirp() at {:p}", (void*) this); }
};

typedef WireCell::Singleton<Foo> OnlyFoo;

int main()
{
    info("First time:");
    Foo* foo1 = &OnlyFoo::Instance();
    Foo* foo2 = &OnlyFoo::Instance();

    Assert(foo1 == foo2);

    OnlyFoo::Instance().chirp();
    info("Second time:");
    OnlyFoo::Instance().chirp();

    Foo* foo3 = &WireCell::Singleton<Foo>::Instance();
    Assert(foo3 == foo1);
}
