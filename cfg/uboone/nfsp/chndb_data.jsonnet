// example OmniChannelNoiseDB configuration for microboone

local wc = import "wirecell.jsonnet";
local handmade = import "chndb-resp.jsonnet";
local params = import "params.jsonnet";
local gen = import "general.jsonnet";

local rms_cuts = import "chndb_rmscut.jsonnet";

{
    anode: wc.tn(gen.anode),

    tick: params.sample_period,

    // This sets the number of frequency-domain bins used in the noise
    // filtering.  It is expected that time-domain waveforms have the
    // same number of samples.
    nsamples: params.frequency_bins,

    // For MicroBooNE, channel groups is a 2D list.  Each element is
    // one group of channels which should be considered together for
    // coherent noise filtering.
    groups: [std.range(g*48, (g+1)*48-1) for g in std.range(0,171)],

    // Externally determined "bad" channels.
    bad: [446] + std.range(7136, 7199) + std.range(7201,7214) + std.range(7216,7263),

    // Overide defaults for specific channels.  If an info is
    // mentioned for a particular channel in multiple objects in this
    // list then last mention wins.
    channel_info: [             

        // First entry provides default channel info across all
        // channels.  Subsequent entries override a subset of channels
        // with a subset of these entries.
        {
            channels: std.range(0, 2400 + 2400 + 3456 - 1),
            nominal_baseline: 2048.0,  // adc count
            gain_correction: 1.0,     // unitless
            response_offset: 0.0,      // ticks?
            pad_window_front: 0,     // ticks?
            pad_window_back: 0,      // ticks?
	    decon_limit: 0.02,
	    decon_limit1: 0.09
	    adc_limit: 15,
            min_rms_cut: 1.0,         // units???
            max_rms_cut: 5.0,         // units???

            // parameter used to make "rcrc" spectrum
            rcrc: 1.0*wc.millisecond,

            // parameters used to make "config" spectrum
            reconfig : {},

            // list to make "noise" spectrum mask
            freqmasks: [],

            // field response waveform to make "response" spectrum.  
            response: {},

        },

        {
            channels: {wpid: wc.WirePlaneId(wc.Ulayer)},
            freqmasks: [
                { value: 1.0, lobin: 0, hibin: $.nsamples-1 },
                { value: 0.0, lobin: 169, hibin: 173 },
                { value: 0.0, lobin: 513, hibin: 516 },
            ],
            /// this will use an average calculated from the anode
            // response: { wpid: wc.WirePlaneId(wc.Ulayer) },
            /// this uses hard-coded waveform.
            response: { waveform: handmade.u_resp, waveformid: wc.Ulayer },
            response_offset: 79,
            pad_window_front: 20,
	    pad_window_back: 10,
	    decon_limit: 0.02,
	    decon_limit1: 0.09,
	    adc_limit: 15,
        },

        {
            channels: {wpid: wc.WirePlaneId(wc.Vlayer)},
            freqmasks: [
                { value: 1.0, lobin: 0, hibin: $.nsamples-1 },
                { value: 0.0, lobin: 169, hibin: 173 },
                { value: 0.0, lobin: 513, hibin: 516 },
            ],
            /// this will use an average calculated from the anode
            // response: { wpid: wc.WirePlaneId(wc.Vlayer) },
            /// this uses hard-coded waveform.
            response: { waveform: handmade.v_resp, waveformid: wc.Vlayer },
            response_offset: 82,
            pad_window_front: 10,
	    pad_window_back: 10,
	    decon_limit: 0.01,
	    decon_limit1: 0.08,	 
	    adc_limit: 15,
        },

        {
            channels: {wpid: wc.WirePlaneId(wc.Wlayer)},
            nominal_baseline: 400.0,
            pad_window_front: 10,
	    pad_window_back: 10,
	    decon_limit: 0.05,
	    decon_limit1: 0.08,
	    adc_limit: 15,
        },

        {                       // special channel
            channels: 2240,
            freqmasks: [
                { value: 1.0, lobin: 0, hibin: $.nsamples-1 },
                { value: 0.0, lobin: 169, hibin: 173 },
                { value: 0.0, lobin: 513, hibin: 516 },
                { value: 0.0, lobin:  17, hibin:  19 },
            ],
        },

        {                       // these are before hardware fix 
            channels: std.range(2016,2095) + std.range(2192,2303) + std.range(2352,2399),
            reconfig: {
                from: {gain:  4.7*wc.mV/wc.fC, shaping: 1.1*wc.us},
                to:   {gain: 14.0*wc.mV/wc.fC, shaping: 2.2*wc.us},
            }
        },
    ] + rms_cuts,
}
    
