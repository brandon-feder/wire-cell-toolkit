// This configures a job for the simplest signal-only simulation where
// all channels use their nominal field responses and there are no
// misconfigured electronics.  It also excludes noise which as a side
// effect means the output frame does not span a rectangular, dense
// area in the space of channel vs tick.  The kinematics here are a
// mixture of Ar39 "blips" and some ideal, straight-line MIP tracks.
//
// Output is to a .npz file of the same name as this file.  Plots can
// be made to do some basic checks with "wirecell-gen plot-sim".

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local io = import "pgrapher/common/fileio.jsonnet";
local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";

local tools = tools_maker(params);

local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";
local sim = sim_maker(params, tools);



local close = {
    tail: wc.point(1000, 0.0, 5000, wc.mm),
    head: wc.point(1030, 0.0, 5000, wc.mm),
};

// The earliest/latest time that activity at the response plane can be readout
local earliest_time = params.sim.ductor.start_time;
local latest_time = earliest_time + params.sim.ductor.readout_time;

local tracklist = [
    {
        //time: -1.6*wc.ms - 10*wc.cm/(1.1*wc.mm/wc.us),
        time: earliest_time,
        charge: -5000,          // negative means per step
        ray: close,
    },
    {
        time: -1.6*wc.ms,
        charge: -5000,          // negative means per step
        ray: close,
    },
    {
        time: 0*wc.ms,
        charge: -5000,          // negative means per step
        ray: close,
    },
    {
        time: latest_time-100*wc.us,
        charge: -5000,          // negative means per step
        ray: close,
    },
];
local output = "wct-sim-ideal-sig.npz";

    
local anode = tools.anodes[0];
//local depos = g.join_sources(g.pnode({type:"DepoMerger", name:"BlipTrackJoiner"}, nin=2, nout=1),
//                             [sim.ar39(), sim.tracks(tracklist)]);
local depos = sim.tracks(tracklist);

local deposio = io.numpy.depos(output);
local drifter = sim.drifter;
local bagger = sim.make_bagger();
local transform = sim.make_depotransform("nominal", anode, tools.pirs[0]);
local digitizer = sim.digitizer(anode, tag="orig");

// stripped down signal processing config
local sp_filters = import "pgrapher/experiment/uboone/sp-filters.jsonnet";
local sp = g.pnode({
    type: "OmnibusSigProc",
    data: {
        anode: wc.tn(tools.anode),
        field_response: wc.tn(tools.field),
        per_chan_resp: "",
        shaping: params.elec.shaping,
        fft_flag: 0,    // 1 is faster but higher memory, 0 is slightly slower but lower memory
        // r_fake_signal_low_th: 300,
        // r_fake_signal_high_th: 600,
    }
}, nin=1,nout=1,uses=[tools.anode, tools.field] + sp_filters);


local frameio = io.numpy.frames(output);

local slicer = g.pnode({
    type: "SumSlicer",
    data: {
        tag: "",
        tick_span: 4,
    },
}, nin=1, nout=1);

local sink = g.pnode({
    type: "SlicesSink",
}, nin=1, nout=0);


local graph = g.pipeline([depos, deposio, drifter, bagger, transform,
                          digitizer, sp,
                          frameio,
                          slicer, sink]);

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

// Finally, the configuration sequence which is emitted.

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio", "WireCellSigProc", "WireCellImg"],
        apps: ["Pgrapher"]
    }
};

[cmdline] + g.uses(graph) + [app]
