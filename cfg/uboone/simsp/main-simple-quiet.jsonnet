// This is a main WCT configuration file.
// It configures a job to run the most simple simulation.
// Kinematics consist of ideal tracks.


local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";
local sim = import "sim.jsonnet";
local dep = import "depos.jsonnet";
local dig = import "digitizers.jsonnet";



local mydepos = dep.wee_twin_tracks;


// This depends very much on the job so define it here instead of in
// some shared file.
local base_saver = {
    data: par.daq {
        filename: "uboone-simple-quiet-simulation.npz",
    },
};

// Insert depo saver between drifter and ductor.
local depo_saver = g.pnode(base_saver { type: "NumpyDepoSaver" }, nin=1,nout=1);
local mysim = g.insert_one(sim.single_quiet,0,
                           depo_saver, depo_saver,
                           name="SimpleQuietSim");


// Aggregate stuff into an output node
local frame_saver = g.pnode(base_saver {type: "NumpyFrameSaver"}, nin=1,nout=1);
local frame_sink = g.pnode({ type: "DumpFrames" }, nin=1);
local myout = g.intern(
    name = "DaqFile",
    innodes = [frame_saver],
    centernodes = [frame_sink],
    edges = [ g.edge(frame_saver, frame_sink)],
);


local graph = g.intern(
    name = "FullGraph",
    innodes = [mydepos],
    outnodes = [myout],
    centernodes = [mysim],
    edges = [
        g.edge(mydepos, mysim),
        g.edge(mysim, myout),
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
