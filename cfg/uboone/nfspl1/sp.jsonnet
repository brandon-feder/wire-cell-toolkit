// -*- js -*-
// This configures the Signal Processing stage of a job.
//


local wc = import "wirecell.jsonnet";
local par = import "params.jsonnet";


// Caveat: this import breaks the purity of this file.  Without it, it
// would be possible to use this configuration for a stand-alone
// wire-cell job.  However, the art job needs to have a "frame saver"
// (a WC/LS integration component) placed just after the SP component.
// This can probably be cleaned up to break the explicit connection
// but for now we take the expedient route.  
local wcls = import "wcls.jsonnet";


local hf = import "hf_filters.jsonnet";
local lf = import "lf_filters.jsonnet";

// Provides access to the per-channel response functions.
local perchanresp = {
    type : "PerChannelResponse",
    data : {
        filename: par.chresp_file,
    },
};


// The signal processing delegates to a slew of frequency domain
// filters which all have configuration.  The OmnibusSigProc
// component "knows" which filters to use because it hard-codes
// their names.  
local filters = [
    hf.gauss.tight,
    hf.gauss.wide,
    hf.weiner.tight.u,
    hf.weiner.tight.v,
    hf.weiner.tight.w,
    hf.weiner.wide.u,
    hf.weiner.wide.v,
    hf.weiner.wide.w,
    hf.wire.induction,
    hf.wire.collection,

    lf.roi.tight,
    lf.roi.tighter,
    lf.roi.loose,
];

// The signal processing is in a single frame filter.
local sigproc = {
    type: "OmnibusSigProc",
    data: {
        // This class has a HUGE set of parameters.  See
        // OmnibusSigProc.h for the list.  For here, for now, we
        // mostly just defer to the hard coded configuration
        // values.  They can be selectively overriddent.  This
        // class also hard codes a slew of SP filter component
        // names which MUST be correct.
    }
};

local rawsplit = {
    type: "FrameSplitter",
    name: "rawsplitter"
};
local sigsplit = {
    type: "FrameSplitter",
    name: "sigsplitter"
};

local chsel = {
    type: "ChannelSelector",
    data: {
        // channels that will get L1SP applied
        channels: std.range(3566,4305),

        // can pass on only the tags of traces that are actually needed.
        tags: ["raw","gauss"]
    }
};

local l1sp = {
    type: "L1SPFilter",
    data: {
        filter: [0.000305453, 0.000978027, 0.00277049, 0.00694322, 0.0153945, 0.0301973, 0.0524048, 0.0804588, 0.109289, 0.131334, 0.139629, 0.131334, 0.109289, 0.0804588, 0.0524048, 0.0301973, 0.0153945, 0.00694322, 0.00277049, 0.000978027, 0.000305453],
        raw_ROI_th_nsigma: 4.2,
        raw_ROI_th_adclimit:  9,
        overall_time_offset : 0,
        collect_time_offset : 3.0,
        roi_pad: 3,
        raw_pad: 15,
        adc_l1_threshold: 6,
        adc_sum_threshold: 160,
        adc_sum_rescaling: 90,
        adc_ratio_threshold: 0.2,
        adc_sum_rescaling_limit : 50,
        l1_seg_length : 120,
        l1_scaling_factor : 500,
        l1_lambda : 5,
        l1_epsilon : 0.05,
        l1_niteration : 100000,
        l1_decon_limit : 100,
        l1_resp_scale : 0.5,
        l1_col_scale : 1.15,
        l1_ind_scale : 0.5,
        peak_threshold : 1000,
        mean_threshold : 500,
        adctag: "raw",                             // trace tag of raw data
        sigtag: "gauss",                           // trace tag of input signal
        outtag: "l1sp",                            // trace tag for output signal
    }
};

// merge the split output from NF ("raw" tag) and just the "gauss"
// from normal SP for input to L1SP
local rawsigmerge = {
    type: "FrameMerger",
    name: "rawsigmerge",
    data: {
        rule: "replace",

        // note: the first two need to match the order of what data is
        // fed to ports 0 and 1 of this component in the pgraph below!
        mergemap: [
            ["raw","raw","raw"],
            ["gauss","gauss","gauss"],
        ],
    }
};

// merge out of L1 ("l1sp" tag) with previously split output from
// regular SP ("wiener" and "gauss" tags).  The "raw" tag is unlikely
// to be found.
local l1merge = {
    type: "FrameMerger",
    name: "l1merge",
    data: {
        rule: "replace",

        // note: the first two need to match the order of what data is
        // fed to ports 0 and 1 of this component in the pgraph below!
        mergemap: [
            ["raw","raw","raw"],
            ["l1sp","gauss","gauss"],
            ["l1sp","wiener","wiener"],
        ],
    }
};


{
    configs: [perchanresp] + filters + [sigproc, rawsplit, sigsplit, chsel, l1sp, rawsigmerge, l1merge],
    
    edges : [
        {
            tail: { node: wc.tn(rawsplit), port:1 },
            head: { node: wc.tn(rawsigmerge), port:1 }
        },
        {
            tail: { node: wc.tn(rawsplit), port:0 },
            head: { node: wc.tn(sigproc) }
        },
        {
            tail: { node: wc.tn(sigproc) },
            head: wcls.thsave ,
        },                      // break the stand-alone purity
        {                       // of this file with this thsave.
            tail: wcls.thsave ,
            head: { node: wc.tn(sigsplit) },
        },
        {
            tail: { node: wc.tn(sigsplit), port:1 },
            head: { node: wc.tn(l1merge), port:1 },
        },
        {
            tail: { node: wc.tn(sigsplit), port:0 },
            head: { node: wc.tn(rawsigmerge), port:0 },
        },
        {
            tail: { node: wc.tn(rawsigmerge) },
            head: { node: wc.tn(chsel) },
        },
        {
            tail: { node: wc.tn(chsel) },
            head: { node: wc.tn(l1sp) },
        },
        {
            tail: { node: wc.tn(l1sp) },
            head: { node: wc.tn(l1merge), port:0 },
        },
    ],
    input : { node: wc.tn(rawsplit) },
    output : { node: wc.tn(l1merge) },


}
