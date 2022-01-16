// This file configures (most) of the WCT components which exist in
// the WC/LS integration boundary.  They are the ones involved in I/O
// between art::Event and WCT data interfaces.  The main component not
// included here is the channel noise database object.  See
// chndb.jsonnet.

local wc = import "wirecell.jsonnet";
local par = import "params.jsonnet";
local gen = import "general.jsonnet";

// This source converts between LArSoft raw::RawDigit and WCT IFrame
// for input to WCT.  It needs to be added to the "inputers" list in
// the FHiCL for WireCellToolkit module.
local source = {
    type: "wclsRawFrameSource",
    data: {
        // used to locate the input raw::RawDigit in the art::Event
        source_label: par.raw_input_label,
        // names the output frame tags and likely must match the next
        // IFrameFilter (eg, that for the NF)
        frame_tags: ["orig"],
        nticks: par.frequency_bins,
    },
};

// One sink of data back to art::Event.  This saves the intermediate
// noise filtered data as raw::RawDigit.  It needs to be added to the
// "outputers" list in the FHiCL for WireCellToolkit.  Note, it has
// both a type and a name.  Add as wclsFrameSaver:nfsaver.
local nf_saver = {
    type: "wclsFrameSaver",
    name: "nfsaver",
    data: {
        anode: gen.anode,
        digitize: true,
        //digitize: false,
        //sparse: true,
        //pedestal_mean: "fiction",
        pedestal_mean: 0.0,
        pedestal_sigma: 1.75,
        frame_tags: ["raw"],
        nticks: par.output_nticks,
        chanmaskmaps: ["bad"],
    }
};

local wcls_charge_scale = 1.0;  // No scaling, handle it in the butcher module

// The threshold array used to be saved as part of the gauss/wiener
// signal sets but we now save them downstream after L1.  We add a
// saver just for them here.  Alternatively the FrameMerger could be
// improved to all this auxiliary data to be handled.
local th_saver = {
    type: "wclsFrameSaver",
    name: "thsaver",
    data: {
        anode: gen.anode,
        digitize: false,
        sparse: true,
        frame_tags: [],
        frame_scale: null,      // this turns off saving frames
        nticks: par.output_nticks,
        summary_tags: ["threshold"], 
        summary_scale: wcls_charge_scale,
    }
};


// Another sink of data back to art::Event.  This saves the final
// signal processed ROIs as recob::Wire.  It needs to be added to the
// "outputers" list in the FHiCL for WireCellToolkit.  Note, it has
// both a type and a name.  Add as wclsFrameSaver:spsaver.
local sp_saver = {
    type: "wclsFrameSaver",
    name: "spsaver",
    data: {
        anode: gen.anode,
        digitize: false,
        sparse: true,
        frame_tags: ["gauss", "wiener"],
        frame_scale: wcls_charge_scale,
        nticks: par.output_nticks,
    }
};

// A final true sink is needed to cap off the graph.
local frame_sink = {
    type: "DumpFrames",
};


{
    // Give the special nodes this file defines as half-edges.
    source: {node: wc.tn(source)},
    nfsave: {node: wc.tn(nf_saver)},
    thsave: {node: wc.tn(th_saver)},
    spsave: {node: wc.tn(sp_saver)},
    sink: {node: wc.tn(frame_sink)},

    // All the configurables.  Don't forget to include this in the
    // .fcl in either the inputers or outputers list of wcls_main
    // tool.
    configs: [source, nf_saver, th_saver, sp_saver, frame_sink],
}

