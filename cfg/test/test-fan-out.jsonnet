local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local multiplicity = 6;
local idents = std.range(0, multiplicity-1);

local source = g.pnode({
    type: "TrackDepos"
}, nin=0, nout=1);

local pipes = [g.pnode({
    name: "drifter%02d" % n,
    type: "Drifter",
}, nin=1, nout=1) for n in idents];

// 1-to-N
local fanout = g.fan.fanout('DepoFanout', pipes, "TestFanout");

// N sinks
local sinks = [g.pnode({
    name: "deposink%02d" % n,
    type: "DumpDepos",
}, nin=1, nout=0) for n in idents];

// 1-to-N-to-null
local capoff = g.intern(innodes=[fanout],
                        centernodes=sinks,
                        edges=[g.edge(fanout, sinks[n], n, 0) for n in idents],
                        name="capoff");

local graph = g.pipeline([source, capoff], "main");

local app = {
  type: 'Pgrapher',
  data: {
    edges: g.edges(graph),
  },
};

g.uses(graph) + [app]
