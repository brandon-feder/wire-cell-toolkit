local params = import "params/chooser.jsonnet";
local wc = import "wirecell.jsonnet";

local wires = {
    type: "WireSchemaFile",
    data: { filename: params.detector.wires }
};
local fields_nominal = {
    type: "FieldResponse",
    data: { filename: params.detector.fields.nominal }
};
local fields_uvground = {
    type: "FieldResponse",
    data: { filename: params.detector.fields.uvground }
};
local fields_vyground = {
    type: "FieldResponse",
    data: { filename: params.detector.fields.vyground }
};
local fields_truth = {
    type: "FieldResponse",
    data: { filename: params.detector.fields.truth }
};

{
    nominal: {
        type : "AnodePlane",    // 
        data : {
            // WIRECELL_PATH will be searched for these files
            wire_schema: wc.tn(wires),
            field_response: wc.tn(fields_nominal),
            ident : 0,
            gain : params.detector.gain,
            shaping : params.detector.shaping,
            postgain: params.detector.postgain,
            readout_time : params.detector.readout_time,
            tick : params.detector.tick,
        }
    },


    uvground : $.nominal {
        name: "uvground",
        data : super.data {
            field_response: wc.tn(fields_uvground),
        }
    },


    vyground : $.nominal {
        name: "vyground",
        data : super.data {
            field_response: wc.tn(fields_vyground),
        }
    },
    

    truth : $.nominal {
        name: "truth",
        data : super.data {
            // fixme: this should really be some special "field"
            // response file which leads to some kind of "true signal
            // waveforms" For now, just use the nominal one as a stand
            // in to let the configuration and machinery be developed.
            field_response: wc.tn(fields_truth),
        }
    },
    

    objects: [wires, fields_nominal, fields_uvground, fields_vyground, fields_truth, $.nominal, $.uvground, $.vyground, $.truth],
}
