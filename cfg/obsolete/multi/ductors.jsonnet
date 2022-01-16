/// Define some unique ductors to match up with unique anodes.

local params = import "params/chooser.jsonnet";
local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";
{
    nominal: {
        type : 'Ductor',
        data : {
            nsigma : params.simulation.nsigma_diffusion_truncation,
            fluctuate : params.simulation.fluctuate,
            start_time: params.simulation.start_time,
            readout_time: params.detector.readout_time,
            drift_speed : params.simulation.drift_speed,
            first_frame_number: params.simulation.start_frame_number,
            anode: wc.tn(anodes.nominal),
        }
    },        

    uvground : $.nominal {
        name:"uvground",
        data : super.data {
            anode: wc.tn(anodes.uvground),
        }
    },

    vyground : $.nominal {
        name:"vyground",
        data : super.data {
            anode: wc.tn(anodes.vyground),
        }
    },


    truth : $.nominal {
        name:"truth",
        data : super.data {
            anode: wc.tn(anodes.truth),
        }
    },

    objects: [$.nominal, $.uvground, $.vyground, $.truth],
}
