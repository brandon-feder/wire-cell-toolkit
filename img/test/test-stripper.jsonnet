// This configures a job for the simplest signal-only simulation where
// all channels use their nominal field responses and there are no
// misconfigured electronics.  It also excludes noise which as a side
// effect means the output frame does not span a rectangular, dense
// area in the space of channel vs tick.  The kinematics here are a
// mixture of Ar39 "blips" and some ideal, straight-line MIP tracks.
//
// Output is to a .npz file of the same name as this file.  Plots can
// be made to do some basic checks with "wirecell-gen plot-sim".


local g = import "pgraph.jsonnet";
local com = import "test-img-common.jsonnet";


local sink = g.pnode({
    type: "StripsSink",
}, nin=1, nout=0);


local graph = g.pipeline([com.prelude("test-stripper.jsonnet"),
                          com.slices, com.stripper, sink]);

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

[com.cmdline] + g.uses(graph) + [app]
