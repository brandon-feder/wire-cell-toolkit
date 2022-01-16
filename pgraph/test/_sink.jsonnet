local wc = import "wirecell.jsonnet";
local anode = import "_anode.jsonnet";
local params = import "_params.jsonnet";

// used for both noise-only, signal-only or both, so just define it
// here instead of making a whole new file....
local digitizer = {
    type: "Digitizer",
    data : params.adc {
        anode: anode.nominal,
    }
};

// cap off the end of the graph
local frame_sink = { type: "DumpFrames" };


{                     // voltage frame sink
    digitizer: digitizer,
    sink: frame_sink
}

