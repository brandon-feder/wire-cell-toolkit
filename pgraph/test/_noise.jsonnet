local wc = import "wirecell.jsonnet";
local params = import "_params.jsonnet";
local anode = import "_anode.jsonnet";

//
// noise simulation parts
//
local static_csdb = {
    type: "StaticChannelStatus",
    name: "urasai",
};

local noise_model = {
    type: "EmpiricalNoiseModel",
    data: {
        anode: anode.nominal,
        spectra_file: params.files.noise,
        chanstat: wc.tn(static_csdb),
        nsamples: params.daq.ticks_per_readout,
    }
};
local noise_source = {
    type: "NoiseSource",
    data: params.daq {
        model: wc.tn(noise_model),
	anode: anode.nominal,
        start_time: params.daq.start_time,
        stop_time: params.daq.stop_time,
        readout_time: params.daq.readout_time,
    }
};

// This is used to add noise to signal.
local frame_summer = {
    type: "FrameSummer",
    data: {
        align: true,
        offset: 0.0*wc.s,
    }
};

{
    input: { node: wc.tn(frame_summer), port:0 },
    edges: [
        {
            tail: { node: wc.tn(noise_source) },
            head: { node: wc.tn(frame_summer), port:1 },
        },
    ],
    output:{ node: wc.tn(frame_summer) },
    cfgseq: [static_csdb, noise_model, noise_source, frame_summer],
}
