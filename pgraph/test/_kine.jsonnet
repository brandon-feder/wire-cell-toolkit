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
local blips = ar39blips;

//
//  Some basic/bogus cosmic rays
// 
local zstart=6500;      // mm
local zend=zstart+1500; // mm
local cosmics = {
    type: "TrackDepos",
    name: "cosmics",
    data: {
        step_size: 1.0 * wc.millimeter,
        tracks: [
            {
                time: 0.0*wc.us,
                charge: -5000, // negative means charge per step
                ray: wc.ray(wc.point(100,50,zstart,wc.mm), wc.point(200,50,zend,wc.mm))
            },
            {
                time: 50.0*wc.us,
                charge: -5000, // negative means charge per step
                ray: wc.ray(wc.point(100,-50,zstart,wc.mm), wc.point(200,-50,zend,wc.mm))
            },
        ]
    }
};


// Join the depos from the various kinematics.  The DepoMergers only
// do 2-to-1 joining so have to use a few.  They don't take any real
// configuration so just name them here to refer to them later.
local joincb = { type: "DepoMerger", name: "CosmicBlipJoiner" };
// local joincbb = { type: "DepoMerger", name: "CBBlipJoiner" };


{
    edges: [
        {
            tail: { node: wc.tn(cosmics) },
            head: { node: wc.tn(joincb), port:0 }
        },
        {
            tail: { node: wc.tn(blips) },
            head: { node: wc.tn(joincb), port:1 }
        },
    ],
    output: { node: wc.tn(joincb)},
    cfgseq: [ joincb, cosmics, blips, ],
}
