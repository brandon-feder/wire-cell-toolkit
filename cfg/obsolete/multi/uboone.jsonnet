local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";
local ductors = import "multi/ductors.jsonnet";


// This defines a chain of chains of rules mapped to ductors.
{                           
    type: "MultiDuctor",
    data: {
        anode : wc.tn(anodes.nominal),
        chains : [
            [
                {           // select based on transverse location
                    ductor: wc.tn(ductors.uvground),
                    rule: "wirebounds",    // select based on wire bounds.
                    args: [ // If depo is in one of the regions then this ductor is applied.
                        // Each region is specified as a range in u, v and w wire index ranges.
                        // Remember wire index starts counting with 0 at edge/corners at negative-most Z.
                        // Endpoints are considered part of the index range.
                        // Total region is the logical AND of all specified wire index ranges.
                        [
                            { plane: 0, min:100, max:200 },
                            { plane: 1, min:300, max:400 },
                        ],
                        [ // All regions in the list or logically ORed together.
                            { plane: 0, min:500, max:600 }, // just in U
                        ],
                    ],
                },

                {           // select based on transverse location
                    ductor: wc.tn(ductors.vyground),
                    rule: "wirebounds",    // select based on wire bounds.
                    args: [             // If depo is in one of the regions then this ductor is applied.
                        // Each region is specified as a range in u, v and w wire index ranges.
                        // Remember wire index starts counting with 0 at edge/corners at negative-most Z.
                        // Endpoints are considered part of the index range.
                        [ // Total region is the logical AND of all specified wire index ranges.
                            { plane: 1, min:800, max:800 },
                            { plane: 2, min:600, max:700 },
                        ],
                    ],
                },

                {   // if nothing above matches, then use this one
                    ductor: wc.tn(ductors.nominal),
                    rule: "bool",
                    args: true,
                }
            ],
            [
                {   // Always run this one
                    ductor: wc.tn(ductors.truth),
                    rule: "bool",
                    args: true,
                },
                // fixme: this needs to be extended to allow rules
                // which will split up depositions by some
                // identifier so that, for example, one may save
                // separately signal from protons, electrons,
                // muons.  Some more info in trello.
            ],
        ]
    }
}
