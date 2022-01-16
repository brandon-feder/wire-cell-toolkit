// This collects configuration related to ductors.

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";


// One ductor for each trio of pirs
local ductors = std.mapWithIndex(function (n, pirs) {
    type: 'Ductor',
    name: 'ductor%d' % n,
    data: par.daq + par.lar + par.sim {
        rng: wc.tn(com.random),
        anode: wc.tn(com.anode),
        pirs: std.map(function(pir) wc.tn(pir), pirs),
    },
    uses: [com.random, com.anode] + pirs,
}, com.pirs);

// NOTE: these assume ultimately the anodes and their ductors are
// ordered [nomina, uvground, vyground]
local uboone_shorted_chain =  [
    {
        ductor: wc.tn(ductors[1]),
        rule: "wirebounds",
        args: [ 
            [ { plane:0, min:296, max:296 } ],
            [ { plane:0, min:298, max:315 } ],
            [ { plane:0, min:317, max:317 } ],
            [ { plane:0, min:319, max:327 } ],
            [ { plane:0, min:336, max:337 } ],
            [ { plane:0, min:343, max:345 } ],
            [ { plane:0, min:348, max:351 } ],
            [ { plane:0, min:376, max:400 } ],
            [ { plane:0, min:410, max:445 } ],
            [ { plane:0, min:447, max:484 } ],
            [ { plane:0, min:501, max:503 } ],
            [ { plane:0, min:505, max:520 } ],
            [ { plane:0, min:522, max:524 } ],
            [ { plane:0, min:536, max:559 } ],
            [ { plane:0, min:561, max:592 } ],
            [ { plane:0, min:595, max:598 } ],
            [ { plane:0, min:600, max:632 } ],
            [ { plane:0, min:634, max:652 } ],
            [ { plane:0, min:654, max:654 } ],
            [ { plane:0, min:656, max:671 } ],
        ],
    },

    {
        ductor: wc.tn(ductors[2]),
        rule: "wirebounds",
        args: [
            [ { plane:2, min:2336, max:2399 } ],
            [ { plane:2, min:2401, max:2414 } ],
            [ { plane:2, min:2416, max:2463 } ],
        ],
    },
    {               // catch all if the above do not match.
        ductor: wc.tn(ductors[0]),
        rule: "bool",
        args: true,
    },
];

local multi_ductor = {
    type: "MultiDuctor",
    data : {
        anode: wc.tn(com.anode),
        continuous: par.sim.continuous,
        chains : [
            uboone_shorted_chain
        ],
    },
    uses: ductors,
};


{
    single : g.pnode(ductors[0], nin=1, nout=1),
    multi : g.pnode(multi_ductor, nin=1, nout=1),
}


