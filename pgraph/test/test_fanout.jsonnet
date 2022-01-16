local wc = import "wirecell.jsonnet";

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph"],
        apps: ["Pgrapher"]
    }
};

local cosmics = {
    type: "TrackDepos",
    name: "cosmics",
    data: {
        step_size: 1.0 * wc.millimeter,
        tracks: [
            {
                time: 0.0*wc.us,
                charge: -5000, // negative means charge per step
                ray: wc.ray(wc.point(100,0,0,wc.mm), wc.point(200,0,0,wc.mm))
            },
        ]
    }
};
local fanout = {
    type: "DepoFanout",
    data: {
        multiplicity: 6,
    }
};

local multiplicity = std.range(0,5);
local sinks = [
    {
        type: "DumpDepos",
        name: "%d" % n,
    } for n in multiplicity ];

local  in_edges = [
    {
        tail: { node: wc.tn(cosmics) },
        head: { node: wc.tn(fanout) }
    },
];

local out_edges = [
{
    tail: { node: wc.tn(fanout), port: n },
    head: { node: wc.tn(sinks[n]) },
} for n in multiplicity ];

local app = {
    type: "Pgrapher",
    data: {
        edges: in_edges + out_edges
    }
};


[cmdline, cosmics, fanout] + sinks + [app]
    
