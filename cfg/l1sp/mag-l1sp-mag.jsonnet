local wc = import "wirecell.jsonnet";

local params = import "params.jsonnet";

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio", "WireCellSigProc"],
        apps: ["Pgrapher"]
    }
};

local random = {
    type: "Random",
    data: {
        generator: "default",
        seeds: [0,1,2,3,4],
    }
};
local utils = [cmdline, random];

local wires = {
    type: "WireSchemaFile",
    data: { filename: params.files.wires }
};
local fields = {
    type: "FieldResponse",
    data: { filename: params.files.fields }
};
local anode = {
    type : "AnodePlane",        // 
    data : params.elec + params.daq {
        ident : 0,
	// field response functions.
        field_response: wc.tn(fields),
	// wire geometry
        wire_schema: wc.tn(wires),
    }
};

local magsource = {
    type: "MagnifySource",
    data: {
        filename: std.extVar("input"),
        frames: ["raw","wiener","gauss"],
        // Map between a channel mask map key name and its TTree for reading.
        cmmtree: [         
        ],
    }
};

local magsink = {
    type: "MagnifySink",
    data: {
//        input_filename: std.extVar("input"),
        output_filename: std.extVar("output"),

	// this best be made consistent
	anode: wc.tn(anode),

        // The list of tags on traces to select groups of traces
        // to form into frames.
        frames: ["raw","wiener","gauss"],

        // The list of summary tags to save as 1D hisograms.
        summaries: [],

        // The evilness includes shunting data directly from input
        // file to output file.  This allows the shunt to be
        // limited to the listed histogram categories.  The
        // possible categories include: orig, raw, decon,
        // threshold, baseline and possibly others.  If the value
        // is left unset or null then all categories known to the
        // code will be shunted.
        shunt: [],

        // Map between a channel mask map key name and its TTree for writing.
        cmmtree: [         
            //["bad", "T_bad"],
            //["lf_noisy", "T_lf"],
        ],

    },
};

local frame_sink = {
    type: "DumpFrames",
};

local mag = [magsource, magsink, fields, wires, anode, frame_sink];

local fsplit = {
    type: "FrameSplitter",
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
        fields: wc.tn(fields),
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

local fmerge = {
    type: "FrameMerger",
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
local flow = [fsplit, chsel, l1sp, fmerge];


local app = {
    type: "Pgrapher",
    data: {
        edges: [
            {
                tail: { node: wc.tn(magsource) },
                head: { node: wc.tn(fsplit) },
            },
            {
                tail: { node: wc.tn(fsplit), port:1 },
                head: { node: wc.tn(fmerge), port:1 },
            },
            {
                tail: { node: wc.tn(fsplit), port:0 },
                head: { node: wc.tn(chsel) },
            },
            {
                tail: { node: wc.tn(chsel) },
                head: { node: wc.tn(l1sp) },
            },
            {
                tail: { node: wc.tn(l1sp) },
                head: { node: wc.tn(fmerge), port:0 },
            },
            {
                tail: { node: wc.tn(fmerge) },
                head: { node: wc.tn(magsink) },
            },
            {
                tail: { node: wc.tn(magsink) },
                head: { node: wc.tn(frame_sink) },
            },
        ],
    }        
};

utils + mag + flow + [app]
