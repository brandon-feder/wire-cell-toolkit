local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

// in shell, do something crazy like:
// depofiles=$(for n in  data/?/?-truthDepo.json; do echo -n '"'$n'"',; done)
// wire-cell -C "depofiles=["$depofiles"]" ....
local depofiles = std.extVar("depofiles");

local depos = g.pnode({
    type: 'BeeDepoSource',
    data: {
        filelist: depofiles
    },
}, nin=0, nout=1);

local sink = g.pnode({ type: 'DumpDepos' }, nin=1, nout=0);

local graph = g.pipeline([depos, sink]);

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio"],
        apps: ["Pgrapher"]
    },
};

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph)
    },
};

[cmdline] + g.uses(graph) + [app]
