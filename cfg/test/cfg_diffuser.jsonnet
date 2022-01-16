local wc = import "wirecell.jsonnet";
local p = import "params.jsonnet";
[
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
]
