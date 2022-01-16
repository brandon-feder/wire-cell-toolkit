local wc = import "wirecell.jsonnet";
local params = import "_params.jsonnet";

// select channels for misconfiguration.
local chselmc = {
    type: "ChannelSelector",
    data: {
        // channels that will get misconfigured corresponding to 
        // Run_No 5881 - 6699 = Ch(1. >=2016 - <=2111 & 2. >=2128 - <=2303 3. >=2320 - <=2383) U plane
        channels: std.range(2016,2111) + std.range(2128,2303) + std.range(2320,2383),
    }
};


// Misconfigure channels
local misconfigure = {
    type: "Misconfigure",
    data : {
        from : {            // original shaping
            gain: params.elec.gain,
            shaping: params.elec.shaping,
        },
        to : {              // target misconfiguration
            gain: 4.7*wc.mV/wc.fC,
            shaping: 1.1*wc.us,
        },
        // tick support for elec. response wave
        nsamples: 50,
        tick: params.daq.ticks_per_readout,
        truncate: true,
    },
};
local fsplit = {                // no config
    type: "FrameSplitter",
};
local fmerge = {
    type: "FrameMerger",
    data: {
        rule: "replace",
        // note: no mergemap means ignore tags.
    }
};

{
    input: { node: wc.tn(fsplit) },
    output: { node: wc.tn(fmerge) },

    edges: [
        {
            tail: { node: wc.tn(fsplit), port:1 },
            head: { node: wc.tn(fmerge), port:1 },
        },
        {
            tail: { node: wc.tn(fsplit), port:0 },
            head: { node: wc.tn(chselmc)},
        },
        {
            tail: { node: wc.tn(chselmc)},
            head: { node: wc.tn(misconfigure)},
        },
        {
            tail: { node: wc.tn(misconfigure)},
            head: { node: wc.tn(fmerge), port:0 },
        },
    ],

    cfgseq: [chselmc, misconfigure, fsplit, fmerge]
}
