// This provides default component configurations for MicroBoone.
// They can be aggregated as-is or extended/customized in the
// top-level configuration file.  They make use of dune global
// parameters.

local params = import "dune/globals.jsonnet";
local wc = import "wirecell.jsonnet";


{
// WIRECELL_PATH will be searched for these files
    wires: {
        type: "WireSchemaFile",
        data: { filename: "pdsp-wires.json.bz2" }
    },
    fields: {
        type: "FieldResponse",
        data: { filename:  "garfield-1d-3planes-21wires-6impacts-dune-v1.json.bz2"}
    },

    anode: {
        type : "AnodePlane", //
        name : "dune-anode-plane", // could leave empty, just testing out explicit name
        data : {
            wire_schema: wc.tn($.wires),
            field_response: wc.tn($.fields)
            ident : 0,
            gain : params.gain,
            shaping : params.shaping,
            postgain: params.postgain,
            readout_time : params.readout,
            tick : params.tick,
        }
    },
    
    // shortcut type+name for below
    anode_tn: self.anode.type + ":" + self.anode.name,

    // Several different depoosition sources based on a dumped set of g4hits.
    // JsonDepoSource treats model "electrons" special as there is no recombination.
    onehitdep : {
        type: 'JsonDepoSource',
        name: "onehitdep",
        data : {
            filename: "onehit.jsonnet",
            model: "electrons",    // take "n" as number of electrons directly.
        }
    },
    brooksdepos : {
        type: 'JsonDepoSource',
        name: "brooksdepos",
        data : {
            filename: "g4tuple-fixed.json.bz2",
            model: "electrons",    // take "n" as number of electrons directly.
        }
    },

    // All other models should use the type of some recombination
    // component and also configure that component.  First, we just
    // list some possible ones and below we select them
    energydeps: {
        type: 'JsonDepoSource',
        name: 'energydeps',
        data: {
            // the g4tuple.json file "q" is in units of MeV.  Multiply by
            // ioniztion "W-value" and a mean 0.7 recombination from
            // http://lar.bnl.gov/properties/#particle-pass
            filename: "g4tuple-fixed.json.bz2",
            model: "MipRecombination",    // take "q" as dE, no dX given
        }
    },
    mip_already_recombinated: {
        type: 'MipRecombination',
        data: {
            Rmip: 1.0,          // in case where source of depos does the recombination
        }
    },

    /* todo:
    birksdeps: {
        type: 'JsonDepoSource',
        name: 'birksdeps',
        data : {
            filename: "g4tuple-qsn.json.bz2",
            model: "birks",     // q is dE, s is dX, apply Birks model.
        }
    },
    boxdeps: {
        type: 'JsonDepoSource',
        name: 'boxdeps',
        data : {
            filename: "g4tuple-qsn.json.bz2",
            model: "box",      // q is dE, s is dX, apply Modified Box model.
        }
    },
    */

    depos: self.brooksdepos,
    recombination: null,

    //depos: self.onehitdep,
    //recombination: self.mip_already_recombinated,

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

    ductor: {
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


    noise : {
        type: "SilentNoise",
        data: {},
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
            Ductor: "Ductor",
            Dissonance: "SilentNoise",
            Digitizer: "Digitizer",
            FrameSink: "DumpFrames",            
        }
    },

}
