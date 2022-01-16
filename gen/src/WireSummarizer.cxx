#include "WireCellGen/WireSummarizer.h"
#include "WireCellGen/WireSummary.h"
#include "WireCellUtil/NamedFactory.h"

using namespace WireCell;

WIRECELL_FACTORY(WireSummarizer, WireCell::WireSummarizer, WireCell::IWireSummarizer)

WireSummarizer::WireSummarizer() {}

WireSummarizer::~WireSummarizer() {}

bool WireSummarizer::operator()(const input_pointer& wires, output_pointer& ws)
{
    if (!wires) {
        ws = nullptr;
        return true;
    }
    ws = output_pointer(new WireSummary(*wires));
    return true;
}
