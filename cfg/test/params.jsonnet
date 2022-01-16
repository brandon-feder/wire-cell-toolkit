local wc = import "wirecell.jsonnet";
{
    wires : {
	plane_dx: 10*wc.mm,
	plane_dy: 1*wc.meter,
	plane_dz: 1*wc.meter,
	u: {
	    pitch: 10*wc.mm,
	    //angle: 60.0*wc.deg,
	    x: 3.75*wc.mm,
	},
	v: {
	    pitch: 10*wc.mm,
	    //angle: -60.0*wc.deg,
	    x: 2.5*wc.mm,
	},
	w: {
	    pitch: 10*wc.mm,
	    //angle: 0.0,
	    x: 1.25*wc.mm,
	},
    },
    // digitization tick
    tick: 0.5*wc.us,
    // wire cell time slice
    timeslice: 4*self["tick"],
    // electron drift velocity
    drift_velocity: 1.6 * wc.mm/wc.us,
}
