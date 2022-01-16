local wc = import "wirecell.jsonnet";
local params = import "_params.jsonnet";


local fields_nominal = {
    type: "FieldResponse",
    name: "nominal",
    data: { filename: params.files.fields[0] }
};
local fields_uvground = {
    type: "FieldResponse",
    name: "uvground",
    data: { filename: params.files.fields[1] }
};
local fields_vyground = {
    type: "FieldResponse",
    name: "vyground",
    data: { filename: params.files.fields[2] }
};

local wires = {
    type: "WireSchemaFile",
    data: { filename: params.files.wires }
};

// One Anode for each universe to be used in MultiDuctor to
// super-impose different field response functions. 
local anode_nominal = {
    type : "AnodePlane",
    name : "nominal",
    data : params.elec + params.daq {
        ident : 0,              // must match what's in wires
        field_response: wc.tn(fields_nominal),
        wire_schema: wc.tn(wires)
    }
};
local anode_uvground = anode_nominal {
    name: "uvground",
    data : super.data {
        field_response: wc.tn(fields_uvground),
    }
};
local anode_vyground = anode_nominal {
    name: "vyground",
    data : super.data {
        field_response: wc.tn(fields_vyground),
    }
};

{
    nominal: wc.tn(anode_nominal),
    uvground: wc.tn(anode_uvground),
    vyground: wc.tn(anode_vyground),
    cfgseq: [wires, fields_nominal, fields_uvground, fields_vyground, anode_nominal, anode_vyground, anode_uvground]
}
