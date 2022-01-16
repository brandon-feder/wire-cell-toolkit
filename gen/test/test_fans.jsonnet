local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph"],
        apps: ["Pgrapher"]
    }
};


local src = g.pnode({
    type: "SilentNoise",
    data: {
        nchannels: 10,
        noutputs: 1,
        traces_tag: "orig",
    },
},nin=0, nout=1);

local fanmult = 3;
local fannums = std.range(0, fanmult-1);

local fanout = g.pnode({
    type: "FrameFanout",
    data: {
        multiplicity: fanmult,
        tag_rules: [
            {
                frame: {
                    ".*": "number%d"%n,
                },
                trace: {
                    // fake doing Nmult SP pipelines
                    "orig": ["wiener", "gauss"],
                }
            } for n in fannums],
    }
}, nin=1, nout=fanmult);

local fanin = g.pnode({
    type: "FrameFanin",
    data: {
        multiplicity: fanmult,
        tag_rules: [
            {
                frame: {
                    ["number%d"%n]: ["output%d"%n, "output"],
                },
                trace: {
                    gauss: "gauss%d"%n,
                    wiener: "wiener%d"%n,
                },
                
            } for n in fannums],
        tags: ["from-pipeline-%d"%n for n in fannums]
    }
}, nin=fanmult, nout=1);


local retagger = g.pnode({
    type: "Retagger",
    data: {
        // Note: retagger keeps tag_rules an array to be like frame fanin/fanout.
        tag_rules: [ {
            // Retagger also handles "frame" and "trace" like fanin/fanout
            // merge separately all traces like gaussN to gauss.
            merge: {            
                "gauss\\d": "gauss",
                "wiener\\d": "wiener",
            }
        }],
    }
}, nin=1, nout=1);

local sink = g.pnode({
    type:"DumpFrames",
}, nin=1, nout=0);

local fanzip = g.intern(name="fanzip",
                        innodes=[fanout],
                        outnodes=[fanin],
                        edges=[g.edge(fanout, fanin, n, n) for n in fannums]);

local graph = g.pipeline([src,fanzip,retagger,sink], name="fanpipe");

local app = {
    type: 'Pgrapher',
    data: {
        edges: g.edges(graph),
    }
};
[cmdline] + g.uses(graph) + [app]

