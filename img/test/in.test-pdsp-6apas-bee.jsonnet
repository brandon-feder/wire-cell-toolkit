// This is a main entry point to configure WCT via wire-cell CLI to
// run simulation, noise filtering, signal
// processing, and imaging using Bee JSON files to provide the depos.


// to test:
// jsonnet --ext-src "depofiles=["$(for n in  data/?/?-truthDepo.json; do echo -n '"'$n'"',; done)"]" thisfile.jsonnet
local depofiles = std.extVar("depofiles");


local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";
local f = import "pgrapher/common/funcs.jsonnet";

local io = import "pgrapher/common/fileio.jsonnet";
local params = import "pgrapher/experiment/pdsp/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";
local sim_maker = import "pgrapher/experiment/pdsp/sim.jsonnet";
local img = import "pgrapher/experiment/pdsp/img.jsonnet";

local sp_maker = import "pgrapher/experiment/pdsp/sp.jsonnet";

local tools = tools_maker(params);

local sim = sim_maker(params, tools);

local sp = sp_maker(params, tools);

local depos = g.pnode({
    type: 'BeeDepoSource',
    data: {
        filelist: depofiles
    },
}, nin=0, nout=1);

local drifter = sim.drifter;
local bagger = sim.make_bagger();

local sim_pipes = sim.splusn_pipelines;
//local sim_pipes = sim.signal_pipelines;

// This makes a per-APA pipeline.  To avoid unneeded merge/split it
// conseptually spans both simulation parts and sigproc+imaging.  To
// "glue" this into data we need something a bit different.
local make_a_pipe = function(ind) {
    local anode = tools.anodes[ind],
    local aname = anode.name,
    ret : g.pipeline([
        sim_pipes[ind], 
        sp.make_sigproc(anode, 'sigproc-'+aname),
        img.slicing(anode, aname),
        img.tiling(anode, aname),
        img.solving(anode, aname),
        img.dump(anode, aname, params.lar.drift_speed),
    ], "sn-sp-img-" + aname)
}.ret;

local perapa_pipes = [ make_a_pipe(ind) for ind in std.range(0, std.length(tools.anodes) - 1)];

local snspimg = f.fansink('DepoSetFanout', perapa_pipes, "snspimg");

local graph = g.pipeline([depos, drifter, bagger, snspimg]);

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio",
                  "WireCellSigProc", "WireCellImg"],
        apps: ["Pgrapher"]
    },
};

// Finally, the configuration sequence which is emitted.

[cmdline] + g.uses(graph) + [app]

