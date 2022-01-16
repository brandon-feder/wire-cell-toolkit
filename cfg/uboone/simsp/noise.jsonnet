// Noise simulation pipeline elements
local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";

local static_csdb = {
    type: "StaticChannelStatus",
    name: "urasai",
};

local noise_model = {
    type: "EmpiricalNoiseModel",
    data: {
        anode: wc.tn(com.anode),
        spectra_file: par.files.noise,
        chanstat: wc.tn(static_csdb),
        nsamples: par.daq.ticks_per_readout,
    },
    uses: [com.anode, static_csdb],
};
local noise_source = g.pnode({
    type: "NoiseSource",
    data: par.daq {
        model: wc.tn(noise_model),
        anode: wc.tn(com.anode),
        rng: wc.tn(com.random),
        start_time: par.daq.start_time,
        stop_time: par.daq.stop_time,
        readout_time: par.daq.readout_time,
    },
    uses: [noise_model, com.anode, com.random],
}, nout=1);
local frame_summer = g.pnode({
    type: "FrameSummer",
    data: {
        align: true,
        offset: 0.0*wc.s,
    }
}, nin=2, nout=1);

{
    // Note, this is a special case of interning for two reasons.  We
    // use the same node for both input and output and we cap off one
    // of the ports (frame_summer:1) with an internal edge.
    nominal: g.intern([frame_summer],[frame_summer],[noise_source],
                      iports=frame_summer.iports[:1],
                      oports=frame_summer.oports,
                      edges=[g.edge(noise_source, frame_summer, 0, 1)],
                      name="NominalNoise"),

}
