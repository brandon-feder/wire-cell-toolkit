// This is a main WCT configuration file.
// It configures a job to run:
//
// - ideal track depos
// - signal sim
// - noise sim
// - noise filter
// - simple sigproc (no L1)


local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";
local sim = import "sim.jsonnet";
local dep = import "depos.jsonnet";
local dig = import "digitizers.jsonnet";
local nf = import "nf.jsonnet";
local sp = import "sp.jsonnet";


local mydepos = dep.wee_twin_tracks;


// How we save some stuff out depends very much on the job so define
// it here instead of in some shared file.
local base_saver = {
    data: par.daq {
        filename: "uboone-simple-noisy-sim-nfsp.npz",
    },
};
local depo_saver = g.pnode(base_saver { type: "NumpyDepoSaver" }, nin=1,nout=1);
local raw_saver = g.pnode(base_saver { type: "NumpyFrameSaver", name: "rawsaver" }, nin=1,nout=1);
local sig_saver = g.pnode(base_saver { type: "NumpyFrameSaver", name: "sigsaver" }, nin=1,nout=1);


// Insert depo saver between drifter and ductor.  Must know that their
// edge is the 0'th.
local mysim = g.insert_one(sim.single_noisy,0,
                           depo_saver, depo_saver,
                           name="SimpleNoisySim");

local mynf = nf.before;
local mysp = sp.sigproc;

local frame_sink = g.pnode({ type: "DumpFrames" }, nin=1);

// Top level, complete graph.
local graph = g.intern(
    name = "FullGraph",
    innodes = [mydepos],
    outnodes = [frame_sink],
    centernodes = [mysim, raw_saver, mynf, mysp, sig_saver],
    edges = [
        g.edge(mydepos, mysim),
        g.edge(mysim, raw_saver),
        g.edge(raw_saver, mynf),
        g.edge(mynf, mysp),
        g.edge(mysp, sig_saver),
        g.edge(sig_saver, frame_sink),
    ]);

// this is a bare inode.
local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    }
};

// final configuration sequence.
[com.cmdline] + g.uses(graph) + [app]
