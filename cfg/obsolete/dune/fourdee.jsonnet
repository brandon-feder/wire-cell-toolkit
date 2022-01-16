local dune = import "dune/components.jsonnet";
local params = import "dune/globals.jsonnet";
local wc = import "wirecell.jsonnet";
[
    {                           // main CLI
	type: "wire-cell",
	data: {
	    plugins: ["WireCellGen", "WireCellSio"],
	    apps: ["FourDee"]
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

    dune.depos,
    dune.recombination,

    // anode needed by drifter, ductor and digitzer, so put first
    dune.wires,
    dune.fields,
    dune.anode,

    dune.drifter,
        
    {
        type: "NoiseSource",
        data: {
            start_time: params.start_time,
            readout_time: params.readout,
            sample_period: params.tick,
            first_frame_number: params.start_frame_number,
            anode: dune.anode_tn,
        }
    },

    dune.ductor,

    if params.digitize then dune.digitizer,

    // output to simple histogram frame sink from sio.
    {
        type: "HistFrameSink",
        data: {
            filename: "dune.root",
            anode: dune.anode_tn,
            units: if params.digitize then 1.0 else wc.uV,
        }
    },

    // The "app" component
    dune.fourdee {
        data : super.data {

            //DepoSource: "TrackDepos",
            DepoSource: dune.depos_tn,

            //Dissonance: "NoiseSource",
            Dissonance: "",

            /// Turning off digitizer saves frame as voltage.  Must
            // configure HistFrameSink's units to match!
            Digitizer: if params.digitize then "Digitizer" else "",
            
            FrameSink: "HistFrameSink",            
        }
    },


]

