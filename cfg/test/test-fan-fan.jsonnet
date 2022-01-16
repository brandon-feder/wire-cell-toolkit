local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local multiplicity = 5;
local idents = std.range(0, multiplicity-1);
local identsquare = std.range(0, multiplicity*multiplicity-1);

local source = g.pnode({
    type: "TrackDepos"
}, nin=0, nout=1);

local layer1 = g.pnode({
    type: "DepoFanout",
    name: "layer1",
}, nin=1, nout=multiplicity);

local layer2s = [g.pnode({
    type: "DepoFanout",
    name: "layer2_%02d" % n,
}, nin=1, nout=multiplicity) for n in idents];

local bigfan = g.intern(innodes = [layer1],
                        outnodes = layer2s,
                        edges = [g.edge(layer1, layer2s[n], n, 0) for n in idents],
                        name="bigfan");

// N*N sinks
local sinks = [g.pnode({
    name: "deposink%02d" % n,
    type: "DumpDepos",
}, nin=1, nout=0) for n in identsquare];

local capoff = g.intern(innodes=[bigfan],
                        centernodes=sinks,
                        edges=[g.edge(bigfan, sinks[n], n, 0) for n in identsquare],
                        name="capoff");

local graph = g.pipeline([source, capoff], "main");

local app = {
  type: 'Pgrapher',
  data: {
    edges: g.edges(graph),
  },
};

g.uses(graph) + [app]
