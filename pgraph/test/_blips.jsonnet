local wc = import "wirecell.jsonnet";
local ar39 = import "ar39.jsonnet";
local v = import "vector.jsonnet";
local params = import "_params.jsonnet";

//
// Define some regions and use them for regions in which to generate Ar39 events
//
local bigbox = {
    local half = v.scale(params.detector.extent, 0.5),
    tail: v.topoint(v.vsub(params.detector.center, half)),
    head: v.topoint(v.vadd(params.detector.center, half)),
};
local lilbox = {
    local half = v.frompoint(wc.point(1,1,1,0.5*wc.cm)),
    tail: v.topoint(v.vsub(params.detector.center, half)),
    head: v.topoint(v.vadd(params.detector.center, half)),
};
local ar39blips = { 
    type: "BlipSource",
    name: "fullrate",
    data: {
	charge: ar39,
	time: {
	    type: "decay",
	    start: params.daq.start_time,
            stop: params.daq.stop_time,
	    activity: params.lar.ar39activity * params.detector.drift_mass,
	},
	position: {
	    type:"box",
            extent: bigbox,
	}
    }
};
local debugblips = { 
    type: "BlipSource",
    name: "lowrate",
    data: {
        charge: { type: "mono", value: 10000 },
	time: {
	    type: "decay",
	    start: params.daq.start_time,
            stop: params.daq.stop_time,
            activity: 1.0/(1*wc.ms), // low rate
	},
	position: {
	    type:"box",
	    extent: lilbox,     // localized
	}
    }
};

{
    ar39: ar39blips,
    debug: debugblips,
}
