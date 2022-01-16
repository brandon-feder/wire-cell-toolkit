local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local multiplicity = 6;
local idents = std.range(0, multiplicity-1);

local sources = [g.pnode({
    type: "SilentNoise",
    name: "framesource%02d" % n,
}, nin=0, nout=1) for n in idents];

local pipes = [g.pnode({
    type: "AddNoise",
    name: "noiseadder%02d" % n,
}, nin=1, nout=1) for n in idents];

// N-to-1
local fanin = g.fan.fanin('FrameFanin', pipes, "TestFanin");

// null-to-N-to-N-to-1
local flange = g.intern(centernodes=sources,
                        outnodes=[fanin],
                        edges=[g.edge(sources[n], fanin, 0, n) for n in idents],
                        name="flange");

local sink = g.pnode({
    name: "framesink",
    type: "DumpFrames",
}, nin=1, nout=0);

local graph = g.pipeline([flange, sink], "main");

local app = {
  type: 'Pgrapher',
  data: {
    edges: g.edges(graph),
  },
};

g.uses(graph) + [app]

