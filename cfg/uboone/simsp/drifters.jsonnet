local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";

{
    simple: g.pnode({
        type: "Drifter",
        data: par.lar + par.sim {
            rng: wc.tn(com.random),
            xregions: [ {
                anode: par.sim.response_plane,
                cathode: par.sim.cathode_plane,
            } ],
        },
        uses: [com.random],
    },nin=1,nout=1),
}
