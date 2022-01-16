#include "WireCellImg/StripesSink.h"

#include "WireCellUtil/NamedFactory.h"

WIRECELL_FACTORY(StripesSink, WireCell::Img::StripesSink, WireCell::IStripeSetSink)

using namespace WireCell;

Img::StripesSink::~StripesSink() {}

bool Img::StripesSink::operator()(const IStripeSet::pointer& ss)
{
    if (!ss) {
        return true;
    }

    const auto stripes = ss->stripes();

    return true;
}
