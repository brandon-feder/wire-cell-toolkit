// This main WCT configuration file configures a job to read in data
// from a Magnify file, run noise filtering, run signal processing and
// save the result to a new Magnify file.
//
// Run like:
//
// $ wire-cell -V detector=uboone \
//             -V input=orig-bl.root \
//             -V evt= EventNo \
//             -V output=orig-bl-nf-sp.root \
//             -c uboone/main/mag-nf-sp-mag.jsonnet
//

local wc = import "wirecell.jsonnet";
local guts = import "uboone/sigproc/omni-nf-sp.jsonnet";
local magnify = import "uboone/io/magnify.jsonnet";

// make local vars for these as we need to reference them a couple times below.
local source = magnify.celltreesource {
    data: super.data {
        frames: ["orig"],
    }
};
local in_sink = magnify.sink {
    name: "in_sink",
    data: super.data {
	frames: ["orig"],
	root_file_mode: "RECREATE",
    },
};
local nf_sink = magnify.sink {
    name: "nf_sink",
    data: super.data {
	frames: ["raw"],
	root_file_mode: "UPDATE",
    },
};
local sp_sink = magnify.sink {
    name: "sp_sink",
    data: super.data {
        frames: ["wiener", "gauss"],
	root_file_mode: "UPDATE",
        shunt:["Trun", "hv_baseline","hu_baseline","hw_baseline"],
        cmmtree: [["bad","T_bad"], ["lf_noisy", "T_lf"]],
        summaries: ["threshold"],
    }
};

// now the main configuration sequence.
[
    {                           // main CLI
        type: "wire-cell",
        data: {
            // fixme: need gen for AnodePlane, best to move that to a 3rd lib
            plugins: ["WireCellGen", "WireCellSigProc", "WireCellSio"],
            apps: ["Omnibus"],
        }
    },

    source,
    in_sink,
    nf_sink,
    sp_sink,

] + guts.config_sequence + [

    {
        type: "Omnibus",
        data: {
            source: wc.tn(source),
            //sink: wc.tn(sink),
            filters: [		// linear chaning of frame filters
		wc.tn(in_sink)
	    ] + std.map(wc.tn, guts.noise_frame_filters) + [
		wc.tn(nf_sink),
	    ] + std.map(wc.tn, guts.sigproc_frame_filters) + [
		wc.tn(sp_sink),
            ],
        }
    },
    
]
