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
                time: 10.0*wc.ns,
                charge: -1,
                ray : wc.ray(wc.point(10,0,0,wc.cm), wc.point(10,0,1,wc.cm))
            },
        ]
    }
};
local beam = {
    type: "TrackDepos",
    name: "beam",
    data: {
        step_size: 1.0 * wc.millimeter,
        tracks: [
            {
                time: 0.0*wc.ns,
                charge: -1,
                ray : wc.ray(wc.point(10,0,0,wc.cm), wc.point(10,0,1,wc.cm))
            },
            {
                time: 20.0*wc.ns,
                charge: -1,
                ray : wc.ray(wc.point(10,0,0,wc.cm), wc.point(10,0,1,wc.cm))
            },
        ]
    }
};
local depojoin = {
    type: "DepoMerger",
};
local sink = {
    type: "DumpDepos",
};
    

local app = {
    type: "Pgrapher",
    data: {
        edges: [
            {
                tail: { node: wc.tn(cosmics) },
                head: { node: wc.tn(depojoin), port:0 }
            },
            {
                tail: { node: wc.tn(beam) },
                head: { node: wc.tn(depojoin), port:1 }
            },
            {
                tail: { node: wc.tn(depojoin) },
                head: { node: wc.tn(sink) }
            },
        ]
    }
};

[ cmdline, cosmics, beam, depojoin, sink, app ]
