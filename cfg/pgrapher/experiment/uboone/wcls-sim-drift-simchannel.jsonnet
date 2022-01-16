// This is a main entry to the WCT configuration for a WC/LS job which
// takes in depos from art::Event, drifts them using the WCT drifter
// and then saves them back to art::Event as SimChannels.  See the
// FHiCL file of the same name as this one.

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

//local u_offset = std.extVar("u_time_offset");
//local v_offset = std.extVar("v_time_offset");
//local y_offset = std.extVar("y_time_offset");

local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";
local tools = tools_maker(params);
local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";
local sim = sim_maker(params, tools);

local wcls_maker = import "pgrapher/ui/wcls/nodes.jsonnet";
local wcls = wcls_maker(params, tools);

local anode = tools.anodes[0]; // BR insert
local rng = tools.random; // BR insert

local wcls_input = {
    depos: wcls.input.depos(name="electrons"),
};

// The "type:name" here must match what is set in the FCL "inputers".
// The name itself doesn't mean anything, but it has to match.
//local wcls_depo_source = g.pnode({
//    type: 'wclsSimDepoSource',
//    name: 'electrons',          // remind that we expect electrons
//    data: {
//        model: "",              // but could calculate electrons ourselves
//        scale: 1.0,             // can apply some arbitrary scale to charge
//	art_label: "plopper",
//	art_instance: "bogus",
//    },
//}, nin=0, nout=1); 

local wcls_simchannel_sink = g.pnode({
    type: 'wclsSimChannelSink',
    name: 'postdrift',
    data: {
        artlabel: "drifted",    // where to save in art::Event
 	anode: wc.tn(anode), 
	rng: wc.tn(rng),
	tick: 0.5*wc.us,
	start_time: -1.6*wc.ms, //0.0*wc.s,
	readout_time: self.tick*9600,
	nsigma: 3.0,
	drift_speed: params.lar.drift_speed,
	uboone_u_to_rp:	100*wc.mm,
	uboone_v_to_rp:	100*wc.mm,
	uboone_y_to_rp:	100*wc.mm,
	u_time_offset: 0.0*wc.us,
	v_time_offset: 0.0*wc.us,
	y_time_offset: 0.0*wc.us,
	use_energy: true,
    },
}, nin=1, nout=1, uses=[tools.anode]);


local graph = g.pipeline([wcls_input.depos, //wcls_depo_source,
//local graph = g.pipeline([wcls_depo_source,
                          sim.drifter,
                          wcls_simchannel_sink,
                          sim.depo_sink]); // note, this sink is noisy

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

// Finally, the configuration sequence which is emitted.

g.uses(graph) + [app]
