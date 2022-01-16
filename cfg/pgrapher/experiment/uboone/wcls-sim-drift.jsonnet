// This is a main entry to the WCT configuration for a WC/LS job which
// takes in depos from art::Event, drifts them using the WCT drifter
// and then saves them back to art::Event as SimChannels.  See the
// FHiCL file of the same name as this one.

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";


local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";
local tools = tools_maker(params);
local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";
local sim = sim_maker(params, tools);

// The "type:name" here must match what is set in the FCL "inputers".
// The name itself doesn't mean anything, but it has to match.
local wcls_depo_source = g.pnode({
    type: 'wclsSimDepoSource',
    name: 'electrons',          // remind that we expect electrons
    data: {
        model: "",              // but could calculate electrons ourselves
        scale: -1.0,            // Can apply some arbitrary scale to
        // charge.  This needs to be negative until this is fixed:
        // https://github.com/WireCell/larwirecell/issues/7
        
    },
}, nin=0, nout=1); 

// Here I make up some stuff
local wcls_simchannel_saver = g.pnode({
    type: 'wclsSimChannelSaver',
    name: 'postdrift',
    data: {
        artlabel: "drifted",    // where to save in art::Event
    },
}, nin=1, nout=1);


local graph = g.pipeline([wcls_depo_source,
                          sim.drifter,
                          wcls_simchannel_saver,
                          sim.depo_sink]); // note, this sink is noisy

local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

// Finally, the configuration sequence which is emitted.

g.uses(graph) + [app]
