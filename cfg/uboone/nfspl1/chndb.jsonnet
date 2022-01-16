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

// maybe used below
local before = if par.noisedb.flavor == "wcls" then wcls_before
          else if par.noisedb.flavor == "wct"  then wct_before;
local after = if par.noisedb.flavor == "wcls" then wcls_after
         else if par.noisedb.flavor == "wct"  then wct_after;

local multi = {
    type: "wclsMultiChannelNoiseDB",
    // note, if a name is given here, it must match what is used in the .fcl for inputers.
    data: {
        rules: [
            {
                rule: "runbefore",
                chndb: wc.tn(before),
                args: par.noisedb.run12boundary
            },
            {
                rule: "runstarting",
                chndb: wc.tn(after),
                args: par.noisedb.run12boundary,
            },
            // note, there might be a need to add a catchall if the
            // above rules are changed to not cover all run numbers.
        ],
    }
};


// finally return what is wanted.
if par.noisedb.epoch == "multi" then
{
    configs : [before, after, multi],
    typename : wc.tn(multi)
}    
else if par.noisedb.epoch == "before" then
{
    configs : [before],
    typename : wc.tn(before),
}
else if par.noisedb.epoch == "after" then
{
    configs : [after],
    typename : wc.tn(after),
}
