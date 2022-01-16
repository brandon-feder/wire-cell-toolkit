local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet"; 
local magnify = import "uboone/io/magnify.jsonnet";
local omni = import "uboone/sigproc/omni.jsonnet";

// make local vars for these as we need to reference them a couple times.
local source = magnify.source {
    data: super.data {
        frames: ["orig"],
        cmmtree: [],            // none
    }
};
local sink = magnify.sink {
    data: super.data {
        frames: ["raw"],
        shunt:["Trun", "hu_orig", "hv_orig", "hw_orig",
               "hv_baseline","hu_baseline","hw_baseline"],
        cmmtree: [["bad","T_bad"], ["lf_noisy", "T_lf"]],
        summaries:[],
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

    anodes.nominal,
    
    omni.noisedb,

    // individual noise filter filters
    omni.channel_filters.bitshift,
    omni.channel_filters.single,
    omni.channel_filters.grouped,
    omni.channel_filters.status,

    // the main noise filter
    omni.noisefilter,

    omni.pmtfilter,

    sink,

    {
        type: "Omnibus",
        data: {
            source: wc.tn(source),
            sink: wc.tn(sink),
            filters: [
                wc.tn(omni.noisefilter),
                wc.tn(omni.pmtfilter),
            ],
        }
    },
    
]
