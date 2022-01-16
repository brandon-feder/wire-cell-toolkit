local wc = import "wirecell.jsonnet";
[
    {
	type:"TrackDepos",
	data: {
	    step_size: 1.0 * wc.millimeter,
	    tracks: [
		{
		    time: 10.0*wc.ns,
		    charge: -1,
		    ray : wc.ray(wc.point(10,0,0,wc.cm), wc.point(100,10,10,wc.cm))
		},
		{
		    time: 120.0*wc.ns,
		    charge: -2,
		    ray : wc.ray(wc.point(2,0,0,wc.cm), wc.point(3, -100,0,wc.cm))
		},
		{
		    time: 99.0*wc.ns,
		    charge: -3,
		    ray : wc.ray(wc.point(130,50,50,wc.cm), wc.point(11,-50,-30,wc.cm))
		}
	    ],
	}
    }
]


