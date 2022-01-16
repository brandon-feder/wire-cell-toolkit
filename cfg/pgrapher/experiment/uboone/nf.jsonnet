// This provides some noise filtering related pnodes, 

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

function(params, tools, chndbobj, name="")
{
    local bitshift = {
        type: "mbADCBitShift",
        name:name,
        data: {
            Number_of_ADC_bits: params.adc.resolution,
            Exam_number_of_ticks_test: 500,
            Threshold_sigma_test: 7.5,
            Threshold_fix: 0.8,
        },
    },
    local status = {
        type: "mbOneChannelStatus",
        name:name,
        data: {
            Threshold: 3.5,
            Window: 5,
            Nbins: 250,
            Cut: 14,
            anode: wc.tn(tools.anode)
        },            
    },
    local single = {
        type: "mbOneChannelNoise",
        name:name,
        data: {
            noisedb: wc.tn(chndbobj),
            anode: wc.tn(tools.anode)
        }
    },
    local grouped = {
        type: "mbCoherentNoiseSub",
        name:name,
        data: {
            noisedb: wc.tn(chndbobj),
            anode: wc.tn(tools.anode)
        }
    },

    local obnf = g.pnode({
        type: "OmnibusNoiseFilter",
        name:name,
        data : {

            // This is the number of bins in various filters
            nsamples : params.nf.nsamples,

            maskmap: { chirp: "bad", noisy: "bad" },
            channel_filters: [
                wc.tn(bitshift),
                wc.tn(single)
            ],
            grouped_filters: [
                wc.tn(grouped),
            ],
            channel_status_filters: [
                wc.tn(status),
            ],
            noisedb: wc.tn(chndbobj),
            intraces: "orig",
            outtraces: "quiet",
        }
    }, uses=[chndbobj, tools.anode, single, grouped, bitshift, status], nin=1, nout=1),

    local pmtfilter = g.pnode({
        type: "OmnibusPMTNoiseFilter",
        name:name,
        data: {
            intraces: "quiet",
            outtraces: "raw",
            anode: wc.tn(tools.anode),
        }
    }, nin=1, nout=1, uses=[tools.anode]),

    pipe:  g.pipeline([obnf, pmtfilter], name=name),
}.pipe

