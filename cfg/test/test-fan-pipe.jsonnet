local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local multiplicity = 6;
local idents = std.range(0, multiplicity-1);

local source = g.pnode({
    type: "TrackDepos"
}, nin=0, nout=1);

local drifts = [g.pnode({
    name: "drifter%02d" % n,
    type: "Drifter",
}, nin=1, nout=1) for n in idents];

local ducts = [g.pnode({
    name: "ductor%02d" % n,
    type: "DepoTransform",
}, nin=1, nout=1) for n in idents];

local pipes = [g.pipeline([drifts[n], ducts[n]]) for n in idents];

local fpf = g.fan.pipe('DepoFanout', pipes, 'FrameFanin');

local sink = g.pnode({
    name: "framesink",
    type: "DumpFrames",
}, nin=1, nout=0);

local graph = g.pipeline([source, fpf, sink], "main");

local app = {
  type: 'Pgrapher',
  data: {
    edges: g.edges(graph),
  },
};

g.uses(graph) + [app]

