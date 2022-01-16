// This Jsonnet file configures WCT and wire-cell command line to run
// the signal processing.  It reads and writes a "magnify" file.

// Example jsonnet command line to test the Jsonnet compilation
// $ jsonnet -J cfg -V detector=uboone -V input=foo.root cfg/uboone/main/magnify-sigproc-magnify.jsonnet

// Full wire-cell command line:
//
// $ wire-cell -V detector=uboone \
//             -V input=nsp_2D_display_3455_0_6.root \
//             -V output=output.root \
//             -c uboone/main/magnify-sigproc-magnify.jsonnet


local wc = import "wirecell.jsonnet";

local anodes = import "multi/anodes.jsonnet";
local bits = import "uboone/sigproc/bits.jsonnet";
local filters = import "uboone/sigproc/filters.jsonnet";
local omni = import "uboone/sigproc/omni.jsonnet";
local magnify = import "uboone/io/magnify.jsonnet";

// make local vars for these as we need to reference them a couple times.
local source = magnify.source {
    data: super.data {
        frames: ["raw"],
        cmmtree: [["bad","T_bad"],
                  ["lf_noisy", "T_lf"]],
    }
};
local sink = magnify.sink {
    data: super.data {
        frames: ["wiener", "gauss"],
        summaries: ["threshold"],
        shunt:["Trun",
               "hu_orig","hv_orig","hw_orig",
               "hu_raw", "hv_raw", "hw_raw",
               "hu_baseline", "hv_baseline", "hw_baseline"],
        cmmtree: [["bad","T_bad"],
                  ["lf_noisy", "T_lf"]],
    }
};

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

    bits.fieldresponse,

] + filters + [

    bits.perchanresp,

    // omni.noisefilter,
    omni.sigproc,

    sink,
    
    {
        type: "Omnibus",
        data: {
            source: wc.tn(source),
            sink: wc.tn(sink),
            filters: [wc.tn(omni.sigproc)],
        }
    },


]
