local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";
local io = import "pgrapher/common/fileio.jsonnet";

local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";
local tools = tools_maker(params);
local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";

local nf_maker = import "pgrapher/experiment/uboone/nf.jsonnet";
local chndb_maker = import "pgrapher/experiment/uboone/chndb.jsonnet";
local sp_maker = import "pgrapher/experiment/uboone/sp.jsonnet";

local magout = "wct-jsondepo-sim-nf-sp.root";
    
local anode = tools.anodes[0];
local sim = sim_maker(params, tools);

// local depofile = "g4tuple-qsn-v1-fixed.json.bz2";
local depofile = "mcssim.json.bz2";

local depos = sim.jsondepos(file=depofile);

local drifter = sim.drifter;

local ductor = sim.signal; // normal + shorted-U + shorted-Y

local miscon = sim.misconfigure(params); // misconfigured channels

local noise_model = sim.make_noise_model(anode, sim.miscfg_csdb);
//local noise_model = sim.make_noise_model(anode, sim.empty_csdb);
local noise = sim.add_noise(noise_model);

local digitizer = sim.digitizer(anode, tag="orig");
local magnifio = g.pnode({
    type: "MagnifySink",
    name: "origmag",
    data: {
        output_filename: magout,
        root_file_mode: "RECREATE",
        frames: ["orig"],
        anode: wc.tn(anode),
	nrebin: 1,
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
	nrebin: 1,
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
	nrebin: 4,
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


// Example of current CelltreeFrameSink
local celltree = g.pnode({
    type: "CelltreeFrameSink",
    name: "thresholdcelltree",
    data: {
        output_filename: "celltree.root",
        root_file_mode: "RECREATE",
        anode: wc.tn(anode),
        summaries: ["threshold"],
        cmmtree: ["bad"],    
        nsamples: 9592,
	}
}, nin=1, nout=1);

local celltree2 = g.pnode({
    type: "CelltreeFrameSink",
    name: "deconcelltree",
    data: {
        output_filename: "celltree.root",
        root_file_mode: "UPDATE",
        anode: wc.tn(anode),
        frames: ["gauss", "wiener"],
        nsamples: 9592,
	nrebin: 4,
    }
}, nin=1, nout=1);


local noise_epoch = "perfect"; // static list of bad channels
//local noise_epoch = "after";
local chndb = chndb_maker(params, tools).wct(noise_epoch);
local nf = nf_maker(params, tools, chndb); // misconfig channel reconfig
local sp = sp_maker(params, tools); // include l1sp

local sink = sim.frame_sink;

local graph = g.pipeline([depos, drifter, ductor, noise, digitizer, magnifio, nf, magnifio2, sp, magnifio3, celltree2, sink]);
// local graph = g.pipeline([depos, drifter, ductor, noise, digitizer, nf, sp, celltree2, sink]);


// break into subpgraph and insert a new node
// unable to access subgraph pnodes directly
// "cheat": type:name labels the pnode
// g.edge_labels()
// MagnifySink to dump "threshold" after normal SigProc
// local graph2 = g.insert_node(graph, g.edge_labels("OmnibusSigProc", "FrameSplitter:sigsplitter"), magnifio4, magnifio4, name="graph2");
local graph2 = g.insert_node(graph, g.edge_labels("OmnibusSigProc", "FrameSplitter:sigsplitter"), celltree, celltree, name="graph2");

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph2),
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
[cmdline] + g.uses(graph2) + [app]
