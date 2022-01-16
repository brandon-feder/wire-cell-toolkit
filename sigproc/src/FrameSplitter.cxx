#include "WireCellSigProc/FrameSplitter.h"

#include "WireCellUtil/NamedFactory.h"
#include "WireCellAux/FrameTools.h"

#include <iostream>

WIRECELL_FACTORY(FrameSplitter, WireCell::SigProc::FrameSplitter, WireCell::IFrameSplitter)

using namespace WireCell::SigProc;

FrameSplitter::FrameSplitter() {}
FrameSplitter::~FrameSplitter() {}

bool FrameSplitter::operator()(const input_pointer& in, output_tuple_type& out)
{
    if (!in) {
        std::cerr << "FrameSplitter: passing on EOS\n";
    }
    else {
        std::cerr << "FrameSplitter: passing on frame: " << in->ident() << ":";
        for (auto tag : in->trace_tags()) {
            auto tt = Aux::tagged_traces(in, tag);
            std::cerr << " " << tag << "[" << tt.size() << "]";
        }
        std::cerr << std::endl;
    }

    get<0>(out) = in;
    get<1>(out) = in;

    return true;
}
