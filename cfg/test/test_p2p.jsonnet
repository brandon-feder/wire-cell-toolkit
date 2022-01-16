local wc = import "wirecell.jsonnet";
local p = import "params.jsonnet";
[
    {
	type: "wire-cell",
	data: {
	    plugins: ["WireCellGen","WireCellApps", "WireCellTbb"],
	    apps: ["TbbFlow"]
	}
    },
    
    {
	// same as in test_tbb_dfp_diffuser.cxx
	type:"TrackDepos",
	data: {
	    step_size: 1.0 * wc.millimeter,
	    tracks: [
		{
		    time: 10.0*wc.us,
		    ray : wc.ray(wc.point(1,0,0,wc.cm), wc.point(2,0,0,wc.cm))
		},
		{
		    time: 20.0*wc.us,
		    ray : wc.ray(wc.point(1,0,0,wc.cm), wc.point(1,0,1,wc.cm))
		},
		{
		    time: 30.0*wc.us,
		    ray : wc.ray(wc.point(1,-1,1,wc.cm), wc.point(1,1,1,wc.cm))
		}
	    ],
	}
    },

    {
	type:"Drifter",
	data: {
	    location: 15*wc.mm,
	    drift_velocity: 2.0*wc.mm/wc.us,
	}
    },


    {
	type:"Diffuser",
	name:"diffuserU",
	data: {
	    pitch_origin: { x:p.wires.u.x, y:0.0, z:0.0 },
	    pitch_direction: { x:0.0, y:-0.866025, z:0.5},
	    pitch_distance: p.wires.u.pitch,
	    timeslice: p.timeslice,
	    timeoffset: 0.0,
	    starttime: p.wires.u.x/p.drift_velocity,
	    drift_velocity: p.drift_velocity,
	    max_sigma_l: 2.5*p.timeslice
	}
    },
    {
	type:"Diffuser",
	name:"diffuserV",
	data: {
	    pitch_origin: { x:p.wires.v.x, y:0.0, z:0.0 },
	    pitch_direction: { x:0.0, y:0.866025, z:0.5 },
	    pitch_distance: p.wires.v.pitch,
	    timeslice: p.timeslice,
	    timeoffset: 0.0,
	    starttime: p.wires.v.x/p.drift_velocity,
	    drift_velocity: p.drift_velocity,
	    max_sigma_l: 2.5*p.timeslice
	}
    },
    {
	type:"Diffuser",
	name:"diffuserW",
	data: {
	    pitch_origin: { x:p.wires.w.x, y:0.0, z:0.0 },
	    pitch_direction: { x:0.0, y:0.0, z:1.0 },
	    pitch_distance: p.wires.w.pitch,
	    timeslice: p.timeslice,
	    timeoffset: 0.0,
	    starttime: p.wires.w.x/p.drift_velocity,
	    drift_velocity: p.drift_velocity,
	    max_sigma_l: 2.5*p.timeslice
	}
    },


    {
	type:"TbbFlow",
	data: {
	    graph:[
		{
		    tail: wc.Node {type:"TrackDepos"},
		    head: wc.Node {type:"Drifter"}
		},
		{
		    tail: wc.Node {type:"Drifter"},
		    head: wc.Node {type:"Diffuser"}
		}
	    ]
	}
    },
    
]


