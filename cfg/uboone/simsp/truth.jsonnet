local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local com = import "common.jsonnet";
local par = import "params.jsonnet";

local fanout = g.pnode({
    type: "DepoFanout",
    data: {
        multiplicity: 2,
    }
}, nin=1, nout=2);


local tductor = g.pnode({
    type: "TruthTraceID",
    data: par.sim + par.daq + par.lar {
        anode: wc.tn(com.anode),        
    }
}, nin=1, nout=1, uses=[com.anode]);

local joiner = g.pnode({
    type: "FrameFanin",
    data: {
        multiplicity: 2,
        tags: ["simulation", "truth"], // needs to match order of fanin edges 
    }
}, nin=2, nout=1);

{
    // Naively this looks it makes a cycle.  It doesn't.
    // depos ->- 0+---+0 ->- frame
    // frame ->- 1+---+1 ->- depos
    // where out:1 should feed to the "real" sim chain which feeds back to in:1.
    // With those plugged, then that leaves in:0 and out:0 to make the result
    // act like a pipeline element.
    patch: g.intern([fanout, joiner], [fanout, joiner], [tductor],
                    iports=[fanout.iports[0], joiner.iports[0]],
                    oports=[joiner.oports[0], fanout.oports[0]],
                    edges = [
                        g.edge(fanout, tductor, 1, 0),
                        g.edge(tductor, joiner, 0, 1),
                    ]),
}
