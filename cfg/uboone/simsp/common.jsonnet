// Configuration for common, shared components.  These are things not
// directly part of a pipeline element and they may not be all used.
// This file's data structure is a free form object.

local wc = import "wirecell.jsonnet";
local params = import "params.jsonnet";


{
    cmdline: {
        type: "wire-cell",
        data: {
            plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio", "WireCellSigProc"],
            apps: ["Pgrapher"]
        }
    },
    random : {
        type: "Random",
        data: {
            generator: "default",
            seeds: [0,1,2,3,4],
        }
    },
    fields : std.mapWithIndex(function (n, fname) {
        type: "FieldResponse",
        name: "field%d"%n,
        data: { filename: fname }
    }, params.files.fields),

    perchanresp : {
        type: "PerChannelResponse",
        data: {
            filename: params.files.chresp,
        }
    },

    wires : {
        type: "WireSchemaFile",
        data: { filename: params.files.wires }
    },

    elec_resp : {
        type: "ColdElecResponse",
        data: {
            shaping: params.elec.shaping,
            gain: params.elec.gain,
            postgain: params.elec.postgain,
            nticks: params.sim.nticks,
            tick: params.sim.tick,
        },
    },

    rc_resp : {
        type: "RCResponse",
        data: {
            width: 1.0*wc.ms,
            nticks: params.sim.nticks,
            tick: params.sim.tick,
        }
    },

    // there is one trio of PIRs (one per plane) for each field response.
    pirs : std.mapWithIndex(function (n, fr) [
        {
            type: "PlaneImpactResponse",
            name : "PIR%splane%d" % [fr.name, plane],
            data : {
                plane: plane,
                tick: params.sim.tick,
                nticks: params.sim.nticks,
                field_response: wc.tn(fr),
                // note twice we give rc so we have rc^2 in the final convolution
                other_responses: [wc.tn($.elec_resp), wc.tn($.rc_resp), wc.tn($.rc_resp)],
            },
            uses: [fr, $.elec_resp, $.rc_resp],
        } for plane in [0,1,2]], $.fields),

    // 0:nominal, 1:uv-grounded, 2:vy-grounded
    anode : {
        type : "AnodePlane",
        data : params.elec + params.daq {
            ident : 0,              // must match what's in wires
            wire_schema: wc.tn($.wires),
            faces : [
                {
                    response: params.sim.response_plane,
                    cathode: params.sim.cathode_plane,
                },
            ],
        },
        uses: [$.wires],
    },

    field: $.fields[0],         // the nominal field


}
