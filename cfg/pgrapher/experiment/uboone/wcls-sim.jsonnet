// This is a WCT configuration file for use in a WC/LS job.  It is
// expected to be named inside a FHiCL configuration.  That
// configuration must supply the names of converter components as
// "depo_source" and "frame_sink" external variables.
//

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";


local params = import "pgrapher/experiment/uboone/simparams.jsonnet";
local tools_maker = import "pgrapher/common/tools.jsonnet";
local tools = tools_maker(params);
local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet";
local sim = sim_maker(params, tools);

local wcls_maker = import "pgrapher/ui/wcls/nodes.jsonnet";
local wcls = wcls_maker(params, tools);

// for dumping numpy array for debugging
local io = import "pgrapher/common/fileio.jsonnet";

local nf_maker = import "pgrapher/experiment/uboone/nf.jsonnet";
local chndb_maker = import "pgrapher/experiment/uboone/chndb.jsonnet";

local sp_maker = import "pgrapher/experiment/uboone/sp.jsonnet";

    
// This gets used as an art::Event tag and the final output frame
// needs to be tagged likewise.  Note, art does not allow some
// characters, in particular: [_-]
local digit_tag = "orig";

// make sure name matches calling FHiCL
local depos = wcls.input.depos(name="");
local frames = wcls.output.digits(name="", tags=[digit_tag]);

local anode = tools.anodes[0];
local drifter = sim.drifter;

// Signal simulation.
//local ductors = sim.make_anode_ductors(anode);
//local md_pipes = sim.multi_ductor_pipes(ductors);
//local ductor = sim.multi_ductor_graph(anode, md_pipes, "mdg");
local signal = sim.signal;

local miscon = sim.misconfigure(params);

// Noise simulation adds to signal.
local noise_model = sim.make_noise_model(anode, sim.empty_csdb);
local noise = sim.add_noise(noise_model);

local digitizer = sim.digitizer(anode, tag=digit_tag);



//local noise_epoch = "perfect";
local noise_epoch = "after";
local chndb = chndb_maker(params, tools).wct(noise_epoch);
local nf = nf_maker(params, tools, chndb);
//local nf_frameio = io.numpy.frames(output, "nfframeio", tags="raw");

local sp = sp_maker(params, tools);
//local sp_frameio = io.numpy.frames(output, "spframeio", tags="gauss");


local sink = sim.frame_sink;

local graph = g.pipeline([depos,
//                          deposio,
                          drifter, signal, miscon, noise, digitizer,
//                          sim_frameio, magnifio,
                          nf,
//                          nf_frameio,
                          sp,
//                          sp_frameio,
                          sink]);


local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph),
    },
};

// Finally, the configuration sequence which is emitted.

g.uses(graph) + [app]
