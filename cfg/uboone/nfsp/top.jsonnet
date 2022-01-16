// This file provides top-level objects which will help to write the
// main configuration sequence.


local wc = import "wirecell.jsonnet";

local gen = import "general.jsonnet";
local nf = import "nf.jsonnet";
local sp = import "sp.jsonnet";


{

    frame_filters: nf.frame_filters + sp.frame_filters,

    sequence: gen.sequence + nf.sequence + sp.sequence,


    // example omnibus config.  This probably should redone in a main
    // config sequence so that a source/sink can be supplied.
    omnibus: {
	type: "Omnibus",
	data: {
	    // source: ...
	    // sink: ...
	    filters: std.map(wc.tn, $.frame_filters),
	},	    
    },



}
