local wc = import "wirecell.jsonnet";
local params = import "_params.jsonnet";
local anode = import "_anode.jsonnet";


//
//  Now, the simulation processing component nodes
//

//
// signal simulation parts
//
local drifter = {
    type : "Drifter",
    data : params.lar + params.sim  {
        anode: anode.nominal,
    }
};

// One ductor for each universe, all identical except for name and the
// coresponding anode.
local ductor_nominal = {
    type : 'Ductor',
    name : 'nominal',
    data : params.daq + params.lar + params.sim {
        continuous: false,
        nsigma : 3,
	anode: anode.nominal,
    }
};
local ductor_uvground = ductor_nominal {
    name : 'uvground',
    data : super.data {
        anode: anode.uvground,
    }
};
local ductor_vyground = ductor_nominal {
    name : 'vyground',
    data : super.data {
        anode: anode.vyground,
    }
};

// The guts of this chain can be generated with:
// $ wirecell-util convert-uboone-wire-regions \
//                 microboone-celltree-wires-v2.1.json.bz2 \
//                 MicroBooNE_ShortedWireList_v2.csv \
//                 foo.json
//
// Copy-paste the plane:0 and plane:2 in uv_ground and vy_ground, respectively
local uboone_ductor_chain = [
    {
        ductor: wc.tn(ductor_uvground),
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
        ductor: wc.tn(ductor_vyground),
        rule: "wirebounds",
        args: [
            [ { plane:2, min:2336, max:2399 } ],
            [ { plane:2, min:2401, max:2414 } ],
            [ { plane:2, min:2416, max:2463 } ],
        ],
    },
    {               // catch all if the above do not match.
        ductor: wc.tn(ductor_nominal),
        rule: "bool",
        args: true,
    },

];

// note, this rule chain is nonphysical and over-simplified to make
// debugging easier.  A track from Z=0 to Z=3mm*500 will pass through:
// N|UV|N|VY|N field response functions where N=nominal
local test_ductor_chain = [
    {
        ductor: wc.tn(ductor_uvground),
        rule: "wirebounds",
        args: [ 
            [
                { plane: 2, min:100, max:200 },
            ],
        ],
    },

    {
        ductor: wc.tn(ductor_vyground),
        rule: "wirebounds",
        args: [
            [
                { plane: 2, min:300, max:400 },
            ],
        ],
    },

    {               // catch all if the above do not match.
        ductor: wc.tn(ductor_nominal),
        rule: "bool",
        args: true,
    },
];


// One multiductor to rull them all.
local multi_ductor = {
    type: "MultiDuctor",
    data : {
        anode: anode.nominal,
        continuous: false,
        chains : [
            //test_ductor_chain,
            uboone_ductor_chain,
        ],
    }
};


{
    input: { node: wc.tn(drifter) },
    edges: [
        {
            tail: { node: wc.tn(drifter) },
            head: { node: wc.tn(multi_ductor) },
        },
    ],
    output: { node:wc.tn(multi_ductor) },
    cfgseq: [drifter, ductor_nominal, ductor_vyground, ductor_uvground, multi_ductor],
}
