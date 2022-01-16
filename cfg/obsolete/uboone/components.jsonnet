// This provides default component configurations for MicroBoone.
// They can be aggregated as-is or extended/customized in the
// top-level configuration file.  They make use of uboone global
// parameters.

local params = import "uboone/globals.jsonnet";
local wc = import "wirecell.jsonnet";
{
    wires: {
        type: "WireSchemaFile",
        data: { filename: "microboone-celltree-wires-v2.json.bz2", }
    },
    fields: {
        type: "FieldResponse",
        data: { filename: "garfield-1d-3planes-21wires-6impacts-v6.json.bz2", }
    },

    anode: {
        type : "AnodePlane", //
        name : "uboone-anode-plane", // could leave empty, just testing out explicit name
        data : {
            // WIRECELL_PATH will be searched for these files
            wire_schema: wc.tn($.wires),
            field_response: wc.tn($.fields),
            ident : 0,
            gain : params.gain,
            shaping : params.shaping,
            rc_constant : params.rc_constant,
            postgain: params.postgain,
            readout_time : params.readout,
            tick : params.tick,
        }
    },
    
    // shortcut type+name for below
    anode_tn: self.anode.type + ":" + self.anode.name,

    onehitdep : {
        type: 'JsonDepoSource',
        name: "onehitdep",
        data : {
            filename: "onehit.jsonnet",
            model: "electrons",    // take "n" as number of electrons directly.
            scale: -1.0,           // multiply by "n"
            // fixme: this really shouldn't be needed.  There is another -1 on the charge lurking somewhere
        }
    },
    electrondeps: {
        type: 'JsonDepoSource',
        name: "electrondeps",
        data : {
            filename: "g4tuple-qsn-v1-fixed.json.bz2",
            model: "electrons",  // take "n" from depo as already in number of electrons
            scale: 1.0,           // multiply by "n"
        }
    },
    depos: self.electrondeps,
    //depos: self.onehitdep,
    depos_tn: self.depos.type + ":" + self.depos.name,

    drifter: {
        type : "Drifter",
        data : {
            anode: $.anode_tn,
            DL : params.DL,
            DT : params.DT,
            lifetime : params.electron_lifetime,
            fluctuate : params.fluctuate,
        }
    },

    signalductor: {
        type : 'Ductor',
        data : {
            nsigma : params.nsigma_diffusion_truncation,
            fluctuate : params.fluctuate,
            start_time: params.start_time,
            readout_time: params.readout,
            drift_speed : params.drift_speed,
            first_frame_number: params.start_frame_number,
            anode: $.anode_tn,
        }
    },

   truthductor: {
       	type : 'Truth',
        data : {
            nsigma : params.nsigma_diffusion_truncation,
            fluctuate : params.fluctuate,
            start_time: params.start_time,
            readout_time: params.readout,
            drift_speed : params.drift_speed,
            first_frame_number: params.start_frame_number,
            anode: $.anode_tn,
	    truth_type: "Unit"
        }
    },        

    // switch for the actual ductor to use
    ductor: self.truthductor,
    //ductor: self.signalductor,

    noise : {
        type: "NoiseSource",
        data: {
            model: "EmpiricalNoiseModel",
            anode: $.anode_tn,
        },
    },


    digitizer : {
        type: "Digitizer",
        data : {
            gain: 1.0,
            baselines: [900*wc.millivolt,900*wc.millivolt,200*wc.millivolt],
            resolution: 12,
            fullscale: [0*wc.volt, 2.0*wc.volt],
            anode: $.anode_tn,
        }
    },

    fourdee : {
        type : 'FourDee',
        data : {
            DepoSource: "TrackDepos",
            Drifter: "Drifter",
	    Ductor: wc.tn($.ductor),
            Dissonance: "NoiseSource",
            Digitizer: "Digitizer",
            FrameSink: "DumpFrames",            
        }
    },

}
