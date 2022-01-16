local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";

// it's kind of long, so put it in its own file
local default_noisedb_cfg = import "omnicndb.jsonnet"; 


// the data objects this file makes available for use elsewhere:
{

    // fixme: add individual channel noise filter configs

    noisedb: {
        type : "OmniChannelNoiseDB",
        data: default_noisedb_cfg,
    },


    basechanfilter : {
        data: {
            anode: wc.tn(anodes.nominal),
            noisedb: wc.tn($.noisedb),
        }
    },

    channel_filters : {

        single: $.basechanfilter { type: "mbOneChannelNoise" },
        grouped: $.basechanfilter { type: "mbCoherentNoiseSub" },
        status:  {
            type: "mbOneChannelStatus",
            data: {
                anode: wc.tn(anodes.nominal),
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
            noisedb: wc.tn($.noisedb),
            intraces: "orig",
            outtraces: "quiet",
        }
    },

    pmtfilter: {
        type: "OmnibusPMTNoiseFilter",
        data: {
            anode: wc.tn(anodes.nominal),
            intraces: "quiet",
            outtraces: "raw",
        }
    },


    sigproc : {
        type: "OmnibusSigProc",
        data: {
            // This class has a HUGE set of parameters.  See
            // OmnibusSigProc.h for the list.  For here, for now, we
            // mostly just defer to the hard coded values.  
            anode: wc.tn(anodes.nominal),
        }
    },

}

