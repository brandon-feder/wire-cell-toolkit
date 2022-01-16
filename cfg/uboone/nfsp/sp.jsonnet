// -*- js -*-

// This file holds configuraiton for components related to the signal processing.

local params = import "params.jsonnet";
local hf = import "hf_filters.jsonnet";
local lf = import "lf_filters.jsonnet";

{
    // Provides access to the per-channel response functions.
    perchanresp : {
        type : "PerChannelResponse",
        data : {
            filename: params.chresp_file,
        },
    },


    // The signal processing delegates to a slew of frequency domain
    // filters which all have configuration.  The OmnibusSigProc
    // component "knows" which filters to use because it hard-codes
    // their names.  
    filters : [
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
    ],

    // The signal processing is in a single frame filter.
    sigproc : {
        type: "OmnibusSigProc",
        data: {
            // This class has a HUGE set of parameters.  See
            // OmnibusSigProc.h for the list.  For here, for now, we
            // mostly just defer to the hard coded configuration
            // values.  They can be selectively overriddent.  This
            // class also hard codes a slew of SP filter component
            // names which MUST be correct.
        }
    },
    
    frame_filters: [$.sigproc],

    sequence: [$.perchanresp] + $.filters + $.frame_filters,
}
