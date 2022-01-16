// see README.org

local wc = import "wirecell.jsonnet";

local par = import "params.jsonnet";

local rms_cuts = import "chndb-rms-cuts.jsonnet";
local base_data = import "chndb-base.jsonnet";

local before_data = base_data {
    channel_info: super.channel_info + rms_cuts.before,
};
local after_data = base_data {
    channel_info: super.channel_info + rms_cuts.after,
};


local wct_before = {
    type: "OmniChannelNoiseDB",
    name: "chndbprehwfix",
    data: before_data,
};
local wct_after = {
    type: "OmniChannelNoiseDB",
    name: "chndbposthwfix",
    data: after_data,
};

local wcls_before = {
    type: "wclsChannelNoiseDB",
    name: "chndbprehwfix",
    data: before_data {
        // Replace any misconfigured channels using larsoft service
        misconfig_channel: {
            policy: "replace",
            from: {gain:  4.7*wc.mV/wc.fC, shaping: 1.1*wc.us},
            to:   {gain: 14.0*wc.mV/wc.fC, shaping: 2.2*wc.us},
        }
    }        
};
local wcls_after = {
    type: "wclsChannelNoiseDB",
    name: "chndbposthwfix",
    data: after_data {
        // Replace any misconfigured channels using larsoft service
        misconfig_channel: {
            policy: "replace",
            from: {gain:  4.7*wc.mV/wc.fC, shaping: 1.1*wc.us},
            to:   {gain: 14.0*wc.mV/wc.fC, shaping: 2.2*wc.us},
        }
    }        
};


local make_multi(bef, aft) = {
    type: "wclsMultiChannelNoiseDB",
    // note, if a name is given here, it must match what is used in the .fcl for inputers.
    data: {
        rules: [
            {
                rule: "runbefore",
                chndb: wc.tn(bef),
                args: par.nf.run12boundary
            },
            {
                rule: "runstarting",
                chndb: wc.tn(aft),
                args: par.nf.run12boundary,
            },
            // note, there might be a need to add a catchall if the
            // above rules are changed to not cover all run numbers.
        ],
    },
    uses: [bef, aft],           // pnode extension
};


{
    before: wct_before,
    after: wct_after,

    // multi requires running in art/larsoft with access to FNAL DBs
    multi: make_multi(wcls_before, wcls_after),
}
