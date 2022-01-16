// This is a main WCT configuration file which produces the
// configuration sequence.  It is intended to be given to the
// wire-cell command line program.  

local wc = import "wirecell.jsonnet";
local top = import "top.jsonnet";
local nf = import "nf.jsonnet";
local sp = import "sp.jsonnet";


// Define a source and some sinks.  Multiple sinks are used in order
// to reuse the MagnifySink to capture different intermediate frames.

local source = {
    type: "MagnifySource",
    data: {
	filename: std.extVar("input"),
	frames: [ "orig" ],
    }
};
local sink = {
    type: "MagnifySink",
    data: {
        input_filename: std.extVar("input"),
        output_filename: std.extVar("output"),
    },
};

local in_sink = sink {
    name: "in_sink",
    data: super.data{
	frames: ["orig"],
        root_file_mode: "RECREATE",
    },
};
local nf_sink = sink {
    name: "nf_sink",
    data: super.data {
        frames: ["raw"],
        root_file_mode: "UPDATE",
    },
};
local sp_sink = sink {
    name: "sp_sink",
    data: super.data {
        frames: ["wiener", "gauss"],
        root_file_mode: "UPDATE",
        shunt:["Trun", "hv_baseline","hu_baseline","hw_baseline"],
        cmmtree: [["bad","T_bad"], ["lf_noisy", "T_lf"]],
        summaries: ["threshold"],
    }
};


//
// The main configuration sequence
//
[
    { 
	type: "wire-cell",
	data: {
	    plugins: ["WireCellGen", "WireCellSigProc", "WireCellSio"],
            apps: ["Omnibus"],
        }
    },

] + [ source, in_sink, nf_sink, sp_sink ] + top.sequence + [

    {
	type: "Omnibus",
	data: {
	    source: wc.tn(source),
	    // sink: ... no explicit sink as we just use filter-sinks
	    filters: [
		wc.tn(in_sink),
	    ] + std.map(wc.tn, nf.frame_filters) + [
		wc.tn(nf_sink),
	    ] + std.map(wc.tn, sp.frame_filters) + [
		wc.tn(sp_sink),
	    ],
	},
    },
]

    
