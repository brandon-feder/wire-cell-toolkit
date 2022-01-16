// -*- js -*-

// This file holds configuration for components for the noise
// filtering.

local wc = import "wirecell.jsonnet";
local params = import "params.jsonnet";
local chndb_data = import "chndb_data.jsonnet"; 

{
    // Provides access to just one field response file.
    fieldresponse : {
        type: "FieldResponse",
        data: {
            filename: params.fields_file,
        }
    },

    // This should likely be overriden/extended when this
    // configuration is used for a WCLS job.
    chndb : {
	type: "OmniChannelNoiseDB",
	data: chndb_data
    },

    // Configure the "channel filter components" which are used by the
    // OmnibusNoiseFilter.  Defined here as an object as they need to
    // be individually referenced elsewhere.
    channel_filters : {

        single: {
	    type: "mbOneChannelNoise",
	    data: {
		noisedb: wc.tn($.chndb)
	    }
	},
        grouped: {
	    type: "mbCoherentNoiseSub",
	    data: {
		noisedb: wc.tn($.chndb),
	    }
	},
        status:  {
            type: "mbOneChannelStatus",
            data: {
                Threshold: 3.5,
                Window: 5,
                Nbins: 250,
                Cut: 14,
            },            
        },
        bitshift: {
            type: "mbADCBitShift",
            data: {
                Number_of_ADC_bits: 12,
                Exam_number_of_ticks_test: 500,
                Threshold_sigma_test: 7.5,
                Threshold_fix: 0.8,
            },
        },                
    },
    // Also defined as a list of just the config objects themselves.
    channel_filter_list: [$.channel_filters[k] for k in std.objectFields($.channel_filters)],
	
    // This is the main noise filter which delegates to the channel
    // filters.  Bring them all together.  This is a frame filter so
    // needs to know intput/output trace tags.
    noisefilter: {
        type: "OmnibusNoiseFilter",
        data : {
            maskmap: { chirp: "bad", noisy: "bad" },
            channel_filters: [
                wc.tn($.channel_filters.bitshift),
                wc.tn($.channel_filters.single)
            ],
            grouped_filters: [
                wc.tn($.channel_filters.grouped),
            ],
            channel_status_filters: [
                wc.tn($.channel_filters.status),
            ],
            noisedb: wc.tn($.chndb),
            intraces: "orig",
            outtraces: "quiet",
        }
    },
    
    
    // There is also a special noise filter for PMT induced noise.
    // This is a frame filter so needs to know intput/output trace
    // tags.
    pmtfilter: {
        type: "OmnibusPMTNoiseFilter",
        data: {
            intraces: "quiet",
            outtraces: "raw",
        }
    },

    frame_filters: [$.noisefilter, $.pmtfilter],

    sequence: [$.fieldresponse, $.chndb] + $.channel_filter_list + $.frame_filters,
}
    

