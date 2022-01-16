// This file defines some pnodes that provide sources of depositions.

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";
local par = import "params.jsonnet";

{
    // Two little tracks
    wee_twin_tracks: g.pnode({
        type: "TrackDepos",
        data: {
            step_size: 1.0 * wc.millimeter,
            tracks: [
                {
                    time: 0.0*wc.us,
                    charge: -5000, // negative means charge per step
                    ray: wc.ray(wc.point(100,50,500,wc.mm), wc.point(110,50,510,wc.mm))
                },
                {
                    time: 50.0*wc.us,
                    charge: -5000, // negative means charge per step
                    ray: wc.ray(wc.point(100,-50,500,wc.mm), wc.point(110,-50,510,wc.mm))
                },
            ]
        }
    }, nout=1),

    big_twin_tracks: g.pnode({
        type: "TrackDepos",
        data: {
            step_size: 1.0 * wc.millimeter,
            tracks: [
                {
                    time: 0.0*wc.us,
                    charge: -5000, // negative means charge per step
                    ray: wc.ray(wc.point(100,50,500,wc.mm), wc.point(110,50,5510,wc.mm))
                },
                {
                    time: 50.0*wc.us,
                    charge: -5000, // negative means charge per step
                    ray: wc.ray(wc.point(100,-50,500,wc.mm), wc.point(110,-50,5510,wc.mm))
                },
            ]
        }
    }, nout=1),

}
