// This configures the Noise Filtering stage of a job.

local wc = import "wirecell.jsonnet";
local par = import "params.jsonnet";
local chndb = import "chndb.jsonnet";


    // Provides access to just one field response file.
local fieldresponse = {
    type: "FieldResponse",
    data: {
        filename: par.fields_file,
    }
};


// Configure the "channel filter components" which are used by the
// OmnibusNoiseFilter.  Defined here as an object as they need to
// be individually referenced elsewhere.
local channel_filters = {
    single: {
	type: "mbOneChannelNoise",
	data: {
	    noisedb: chndb.typename,
	}
    },
    grouped: {
	type: "mbCoherentNoiseSub",
	data: {
	    noisedb: chndb.typename,
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
};

// Also defined as a list of just the config objects themselves.
local channel_filter_list = [channel_filters[k] for k in std.objectFields(channel_filters)];
	
// This is the main noise filter which delegates to the channel
// filters.  Bring them all together.  This is a frame filter so
// needs to know intput/output trace tags.
local noisefilter = {
    type: "OmnibusNoiseFilter",
    data : {
        maskmap: { chirp: "bad", noisy: "bad" },
        channel_filters: [
            wc.tn(channel_filters.bitshift),
            wc.tn(channel_filters.single)
        ],
        grouped_filters: [
            wc.tn(channel_filters.grouped),
        ],
        channel_status_filters: [
            wc.tn(channel_filters.status),
        ],
        noisedb: chndb.typename,
        intraces: "orig",
        outtraces: "quiet",
    }
};
    
    
// There is also a special noise filter for PMT induced noise.
// This is a frame filter so needs to know intput/output trace
// tags.
local pmtfilter = {
    type: "OmnibusPMTNoiseFilter",
    data: {
        intraces: "quiet",
        outtraces: "raw",
    }
};

// edges, input, output, configs
{
    configs : [fieldresponse] + channel_filter_list + [noisefilter, pmtfilter],

    edges: [
        {
            tail: { node: wc.tn(noisefilter) },
            head: { node: wc.tn(pmtfilter) },
        },
    ],

    input: { node: wc.tn(noisefilter) },

    output: { node: wc.tn(pmtfilter) },
}
    

