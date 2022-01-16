local uboone = import "uboone/components.jsonnet";
local params = import "uboone/globals.jsonnet";
local wc = import "wirecell.jsonnet";
[
    {                           // main CLI
	type: "wire-cell",
	data: {
	    plugins: ["WireCellGen", "WireCellSio"],
	    apps: ["FourDee"]
	}
    },

    {
        type: "Random",
        data: {
            generator: "default",
            seeds: [0,1,2,3,4],
        }
    },

    // "input" is to generate deposition along some track
    {
        type: 'TrackDepos',
        data : {
            step_size : 1*wc.mm,
            tracks : [{
                time: 0.0*wc.ms,
                // if negative, then charge per depo
                // o.w. it's total charge made by track.
                charge: -10000.0*wc.eplus,
                ray: wc.ray(wc.point(101,0,1,wc.mm), wc.point(102,0,1,wc.mm))
            }]
        }
    },

    uboone.depos,

    // anode needed by drifter, ductor and digitzer, so put first
    uboone.wires,
    uboone.fields,
    uboone.anode,

    uboone.drifter,
        
    {
        type: "StaticChannelStatus",
        data: {
            nominal_gain: 14.0*wc.mV/wc.fC,
            nominal_shaping: 2.0*wc.us,
            deviants: [
                {chid: 100, gain: 4.7*wc.mV/wc.fC, shaping: 1.0*wc.us},
                {chid: 101, gain: 4.7*wc.mV/wc.fC, shaping: 1.0*wc.us},
                {chid: 102, gain: 4.7*wc.mV/wc.fC, shaping: 1.0*wc.us},
                {chid: 103, gain: 4.7*wc.mV/wc.fC, shaping: 1.0*wc.us},
            ],
        }
    },
    
    {
        type: "EmpiricalNoiseModel",
        data: {
            spectra_file: "microboone-noise-spectra-v2.json.bz2",
            chanstat: "StaticChannelStatus",
            anode: uboone.anode_tn,
        }
    },

    {
        type: "NoiseSource",
        data: {
            start_time: params.start_time,
            readout_time: params.readout,
            sample_period: params.tick,
            first_frame_number: params.start_frame_number,
            anode: uboone.anode_tn,
            model: "EmpiricalNoiseModel",
        }
    },

    uboone.ductor,

    if params.digitize then uboone.digitizer,

    // output to simple histogram frame sink from sio.
    {
        type: "HistFrameSink",
        data: {
            filename: "uboone.root",
            anode: uboone.anode_tn,
            units: if params.digitize then 1.0 else wc.uV,
        }
    },

    // output to Celltree (TClonesArray) frame sink from sio.
    {
        type: "CelltreeFrameSink",
        data: {
            filename: "uboone.root",
            anode: uboone.anode_tn,
            units: if params.digitize then 1.0 else wc.uV,
            readout_time: params.readout,
        }
    },


    // The "app" component
    uboone.fourdee {
        data : super.data {

            //DepoSource: "TrackDepos",
            DepoSource: uboone.depos_tn,

            Dissonance: "NoiseSource",

            /// Turning off digitizer saves frame as voltage.  Must
            // configure HistFrameSink's units to match!
            Digitizer: if params.digitize then "Digitizer" else "",
            
            FrameSink: "HistFrameSink",            
            //FrameSink: "CelltreeFrameSink",            
        }
    },


]

