// see README.org

local wc = import "wirecell.jsonnet";

local base = import "chndb-base.jsonnet";
local perfect = import "chndb-perfect.jsonnet";
local rms_cuts = import "chndb-rms-cuts.jsonnet";

function(params, tools)
{
    wct: function(epoch="before") {
        type: "OmniChannelNoiseDB",
        name: "ocndb%s"%epoch,
        data :
        if epoch == "perfect"
        then perfect(params, tools.anode, tools.field)
        else base(params, tools.anode, tools.field, rms_cuts[epoch]),
        uses: [tools.anode, tools.field],    // pnode extension
    },

    wcls: function(epoch="before") {
        type: "wclsChannelNoiseDB",
        name: "wclscndb%s"%epoch,
        data : base(params, tools.anode, tools.field, rms_cuts[epoch]) {
            misconfig_channel: {
                policy: "replace",
                from: {gain:  params.nf.misconfigured.gain,
                       shaping: params.nf.misconfigured.shaping},
                to:   {gain: params.elec.gain,
                       shaping: params.elec.shaping},
            },
        },
        uses: [tools.anode, tools.field],    // pnode extension
    },

    wcls_multi: function(name="") {
        local bef = $.wcls("before"),
        local aft = $.wcls("after"),
        type: "wclsMultiChannelNoiseDB",
        name: name,
        data: {
            rules: [
                {
                    rule: "runbefore",
                    chndb: wc.tn(bef),
                    args: params.nf.run12boundary
                },
                {
                    rule: "runstarting",
                    chndb: wc.tn(aft),
                    args: params.nf.run12boundary,
                },
                // note, there might be a need to add a catchall if the
                // above rules are changed to not cover all run numbers.
            ],
        },
        uses: [bef, aft],           // pnode extension
    },

}

