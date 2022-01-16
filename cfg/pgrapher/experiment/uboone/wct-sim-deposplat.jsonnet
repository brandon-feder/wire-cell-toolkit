// This configures a WCT job for a pipeline that includes full MB
// signal and noise effects in the simulation. and the NF/SP
// components to correct them.  The kinematics here are a mixture of
// Ar39 "blips" and ideal, straight-line MIP tracks.
//
// In particular, there are three "PlaneImpactResponse" objects.
// Which one used depends on the given channel or wire being "shorted"
// or not.
//
// Output is to a .npz file of the same name as this file.  Plots can
// be made to do some basic checks with "wirecell-gen plot-sim".

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local io = import "pgrapher/common/fileio.jsonnet";

local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
// customized override -- moved to simparams.jsonnet
/* local params = default_params{ */
/*     files: super.files{ */
/*         chresp: null, */
/*     } */
/* }; */

local tools_maker = import "pgrapher/common/tools.jsonnet";

local tools = tools_maker(params);

local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";

local nf_maker = import "pgrapher/experiment/uboone/nf.jsonnet";
local chndb_maker = import "pgrapher/experiment/uboone/chndb.jsonnet";

local sp_maker = import "pgrapher/experiment/uboone/sp.jsonnet";

local stubby = {
    tail: wc.point(1000, -1000, 5000.0, wc.mm),
    head: wc.point(1500, -1000, 10360.0, wc.mm),
};

local tracklist = [
    {
        time: 0.0*wc.ms,
        charge: -5000,          // negative means per step
        ray: stubby,
        //ray: params.det.bounds,
    },
];

local tracklist2 = [
    { // shorted V plane
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, 90-86.6, 7200-50, wc.mm), wc.point(1000, 90+86.6, 7200+50, wc.mm)),
    },
    { // shorted U plane
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, -90-86.6, 7200+50, wc.mm), wc.point(1000, -90+86.6, 7200-50, wc.mm)),
    },
    { // normal V
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, 90-86.6, 6200-50, wc.mm), wc.point(1000, 90+86.6, 6200+50, wc.mm)), 
    },
    { // normal U
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, -90-86.6, 6200+50, wc.mm), wc.point(1000, -90+86.6, 6200-50, wc.mm)), 
    },
];

local tracklist3 = [
    { // V plane (0-100, 2000-86.6*2=1826.8)
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, -100-86.6, 1826.8-50, wc.mm), wc.point(1000, -100+86.6, 1826.8+50, wc.mm)),
    },
    { // Y plane
        time: 1*wc.ms,
        charge: -20000,          // negative means per step
        ray: wc.ray(wc.point(1000, 0, 2000-100, wc.mm), wc.point(1000, 0, 2000+100, wc.mm)),
    },
];

local output = "wct-sim-ideal-sn-nf-sp.npz";
local magout = "wct-sim-ideal-sn-nf-sp.root";
    
local anode = tools.anodes[0];

local sim = sim_maker(params, tools);

//local depos = g.join_sources(g.pnode({type:"DepoMerger", name:"BlipTrackJoiner"}, nin=2, nout=1),
//                             [sim.ar39(), sim.tracks(tracklist)]);
local depos = sim.tracks(tracklist);


local deposio = io.numpy.depos(output);

local drifter = sim.drifter;

//local ductors = sim.make_anode_ductors(anode);
//local md_chain = sim.multi_ductor_chain(ductors);
//local ductor = sim.multi_ductor(anode, ductors, [md_chain]);
//local md_pipes = sim.multi_ductor_pipes(ductors);
//local ductor = sim.multi_ductor_graph(anode, md_pipes, "mdg");
//local ductor = sim.make_ductor("nominal", anode, tools.pirs[0]);
// local ductor = sim.signal;
local deposplat = sim.make_ductor('splat', anode, tools.pirs[0], 'DepoSplat');


local noise_model = sim.make_noise_model(anode, sim.miscfg_csdb);
//local noise_model = sim.make_noise_model(anode, sim.empty_csdb);
//local noise = sim.noise_source_and_sum(anode, noise_model).return;
local noise = sim.add_noise(noise_model);

local digitizer = sim.digitizer(anode, tag="orig");
local sim_frameio = io.numpy.frames(output, "simframeio", tags="orig");
local magnifio = g.pnode({
    type: "MagnifySink",
    name: "origmag",
    data: {
        output_filename: magout,
        root_file_mode: "RECREATE",
        frames: ["orig"],
        anode: wc.tn(anode),
    },
}, nin=1, nout=1);

local magnifio2 = g.pnode({
    type: "MagnifySink",
    name: "rawmag",
    data: {
        output_filename: magout,
        root_file_mode: "UPDATE",
        frames: ["raw"],
        cmmtree: [["bad", "T_bad"], ["lf_noisy", "T_lf"]], // maskmap in nf.jsonnet 
        anode: wc.tn(anode),
    },
}, nin=1, nout=1);

local magnifio3 = g.pnode({
    type: "MagnifySink",
    name: "deconmag",
    data: {
        output_filename: magout,
        root_file_mode: "UPDATE",
        frames: ["gauss", "wiener"],
        //cmmtree: [["bad", "T_bad"], ["lf_noisy", "T_lf"]], 
        anode: wc.tn(anode),
        //summaries: ["threshold"], 
        // not saved in FrameMerger
        // a dedicated FrameSaver breaks into the subpgraph
        // using g.insert_node()
    },
}, nin=1, nout=1);

local ctreeio3 = g.pnode({
    type: "CelltreeFrameSink",
    name: "deconctree",
    data: {
        output_filename: magout,
        root_file_mode: "UPDATE",
        frames: ["gauss", "wiener"],
        //cmmtree: [["bad", "T_bad"], ["lf_noisy", "T_lf"]], 
        anode: wc.tn(anode),
        nsamples: 9600,
        //summaries: ["threshold"], 
        // not saved in FrameMerger
        // a dedicated FrameSaver breaks into the subpgraph
        // using g.insert_node()
    },
}, nin=1, nout=1);

local magnifio4 = g.pnode({
    type: "MagnifySink",
    name: "thresholdmag",
    data: {
        output_filename: magout,
        root_file_mode: "UPDATE",
        anode: wc.tn(anode),
        summaries: ["threshold"],
    },
}, nin=1, nout=1);

local noise_epoch = "perfect";
//local noise_epoch = "after";
local chndb = chndb_maker(params, tools).wct(noise_epoch);
local nf = nf_maker(params, tools, chndb);
local nf_frameio = io.numpy.frames(output, "nfframeio", tags="raw");

//misconfigured channels from database
//local miscon = sim.misconfigure(params, chndb);

local miscon = sim.misconfigure(params);

local sp = sp_maker(params, tools);
local sp_frameio = io.numpy.frames(output, "spframeio", tags="gauss");

local sink = sim.frame_sink;

local retagger = g.pnode({
    type: 'Retagger',
    data: {
        // Note: retagger keeps tag_rules in an array to be like frame
        // fanin/fanout but only uses first element.
        tag_rules: [{
            // Retagger also handles "frame" and "trace" like
            // fanin/fanout and also "merge" separately all traces
            // like gaussN to gauss.
            frame: {
                '.*': 'gauss', // celltree io needs input tag: "gauss/wiener/orig"
            },
            merge: {
                'gauss\\d': 'gauss',
                'wiener\\d': 'wiener',
            },
        }],
    },
}, nin=1, nout=1);

// local graph = g.pipeline([depos, drifter, ductor, miscon, noise, digitizer, magnifio, nf, magnifio2, sp, ctreeio3, magnifio3, sink]);
local graph = g.pipeline([depos, drifter, deposplat, retagger, ctreeio3, magnifio3, sink]);
//local graph = g.pipeline([depos, drifter, ductor, noise, digitizer, magnifio, nf, magnifio2, sink]);


// break into subpgraph and insert a new node
// unable to access subgraph pnodes directly
// "cheat": type:name labels the pnode
// g.edge_labels()
// MagnifySink to dump "threshold" after normal SigProc
// local graph2 = g.insert_node(graph, g.edge_labels("OmnibusSigProc", "FrameSplitter:sigsplitter"), magnifio4, magnifio4, name="graph2");

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph /*graph2*/),
    },
};

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio", "WireCellSigProc", "WireCellRoot"],
        apps: ["Pgrapher"]
    }
};


// Finally, the configuration sequence which is emitted.
[cmdline] + g.uses(graph /*graph2*/) + [app]
