// This is a main Wire Cell Toolkit configuration file for use with
// art/LArSoft.
//
// It configures WCT to run inside LArSoft in order to run MicroBooNE
// noise filtering (NF) and signal processing (SP).
//
// Most of the configuration is provided as part of WCT as located by
// directories in a WIRECELL_PATH environment variable.
//
// To compile to JSON for production running at FNAL run something like:
//
//   jsonnet -J /path/to/wire-cell-cfg \
//     -V noisedb=fnal \
//     -V hwnf_epoch=before \
//      /path/to/wire-cell-cfg/uboone/nfsp/ubnfsp.jsonnet | bzip2 > ubnfsp-before.json.bz2 
//
// Repeat with "hwnf_epoch=after".
// 
// The noisedb can be set to "static" to test on computers lacking
// access to FNAL DB.




// These are provided by the wire-cell-cfg package.
local wc = import "wirecell.jsonnet";
local gen = import "general.jsonnet";
local static_nf = import "nf.jsonnet";
local sp = import "sp.jsonnet";
local params = import "params.jsonnet";
local chndb_data = import "chndb_data.jsonnet"; 


// Override parts of the NF config in order to use LArSoft-specific
// channel noise database class.
local nf = (if std.extVar("noisedb") == "static" then static_nf else static_nf {
    chndb : {
	type: "wclsChannelNoiseDB",
	data: chndb_data {
            anode: wc.tn(gen.anode),
	    // don't set bad_channel policy.  NF determines bad channels internally
	    // bad_channel: { policy: "replace" },
	    misconfig_channel: {
                policy: "replace",
                from: {gain:  4.7*wc.mV/wc.fC, shaping: 1.1*wc.us},
                to:   {gain: 14.0*wc.mV/wc.fC, shaping: 2.2*wc.us},
	    }
	}
    }
});



// This source converts between LArSoft raw::RawDigit and WCT IFrame
// for input to WCT.  It needs to be added to the "inputers" list in
// the FHiCL for WireCellToolkit module.
local source = {
    type: "wclsRawFrameSource",
    data: {
	// used to locate the input raw::RawDigit in the art::Event
        source_label: "daq",
	// names the output frame tags and likely must match the next
	// IFrameFilter (eg, that for the NF)
        frame_tags: ["orig"],
	nticks: params.frequency_bins,
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
        anode: wc.tn(gen.anode),
        digitize: true,
        //digitize: false,
        //sparse: true,
        //pedestal_mean: "fiction",
        pedestal_mean: 0.0,
        pedestal_sigma: 1.75,
        frame_tags: ["raw"],
        nticks: params.output_nticks,
        chanmaskmaps: ["bad"],
    }
};



//Additional(Optional-Check) sink of data back to art:Event. This is for
//saving noise-filtered output in recob:wire i.e, float. Add as wclsFrameSaver:nfrsaver
local nfr_saver = {
    type: "wclsFrameSaver",
    name: "nfrsaver",
    data: {
        anode: wc.tn(gen.anode),
        //digitize: true,
        digitize: false,
        sparse: false,
        //pedestal_mean: "fiction",
        pedestal_mean: 0.0,
        pedestal_sigma: 1.75,
        frame_tags: ["raw"],
        nticks: params.output_nticks,
        chanmaskmaps: [],
    }
};


// Another sink of data back to art::Event.  This saves the final
// signal processed ROIs as recob::Wire.  It needs to be added to the
// "outputers" list in the FHiCL for WireCellToolkit.  Note, it has
// both a type and a name.  Add as wclsFrameSaver:spsaver.
local wcls_charge_scale = 1.0;  // No scaling, handle it in the butcher module
local sp_saver = {
    type: "wclsFrameSaver",
    name: "spsaver",
    data: {
        anode: wc.tn(gen.anode),
        digitize: false,
        sparse: true,
        frame_tags: ["gauss", "wiener"],
        frame_scale: wcls_charge_scale,
        nticks: params.output_nticks,
        summary_tags: ["threshold"], 
        summary_scale: wcls_charge_scale,
    }
};




// Finally, the main config sequence:

[ gen.anode, source, nf_saver, nfr_saver, sp_saver ] + nf.sequence + sp.sequence + [

    {
        type: "Omnibus",
        data: {
            source: wc.tn(source),
            //sink: wc.tn(sink),
            filters: std.map(wc.tn, nf.frame_filters + [nf_saver,nfr_saver] + sp.frame_filters + [sp_saver]),
        }
    },
    
]    

