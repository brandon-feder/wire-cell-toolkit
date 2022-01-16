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
local sp_filters = import "pgrapher/experiment/uboone/sp-filters.jsonnet";
{
    local anode = tools.anodes[0],

    prelude(output = "test-img.npz") :: {
        local depos = sim.tracks(tracklist),
        local deposio = io.numpy.depos(output),
        local drifter = sim.drifter,
        local bagger = sim.make_bagger(),
        local transform = sim.make_depotransform("nominal", anode, tools.pirs[0]),
        local digitizer = sim.digitizer(anode, tag="orig"),
        local sp = g.pnode({
            type: "OmnibusSigProc",
            data: {
                anode: wc.tn(anode),
                field_response: wc.tn(tools.field),
                per_chan_resp: "",
                shaping: params.elec.shaping,
                fft_flag: 0,    // 1 is faster but higher memory, 0 is slightly slower but lower memory
                // r_fake_signal_low_th: 300,
                // r_fake_signal_high_th: 600,
            }
        }, nin=1,nout=1,uses=[anode, tools.field] + sp_filters),
        local frameio = io.numpy.frames(output),

        ret: g.pipeline([depos, deposio, drifter, bagger, transform, digitizer, sp, frameio]),
    }.ret,

    slicer : g.pnode({
        type: "SumSlicer",
        data: {
            tag: "",
            tick_span: 4,
            anode: wc.tn(anode),
        },
    }, nin=1, nout=1, uses=[anode]),

    slices : g.pnode({
        type: "SumSlices",
        data: {
            tag: "",
            tick_span: 4,
            anode: wc.tn(anode),
        },
    }, nin=1, nout=1, uses=[anode]),

    stripper : g.pnode({
        type: "NaiveStripper",
    }, nin=1, nout=1),


    cmdline : {
        type: "wire-cell",
        data: {
            plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio", "WireCellSigProc", "WireCellImg"],
            apps: ["Pgrapher"]
        },
    }
}



