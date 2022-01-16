// This is a WCT configuration file for use in a WC/LS simulation job.
// 
// It is expected to be named inside a FHiCL configuration.  The names
// for the "inputer" and "outputer" converter components MUST match
// what is used here in wcls_input and wcls_output objects.
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

    
// This tags the output frame of the WCT simulation and is used in a
// couple places so define it once.
local sim_adc_frame_tag = "orig";

// Collect the WC/LS input converters for use below.  Make sure the
// "name" matches what is used in the FHiCL that loads this file.
// art_label (producer, e.g. plopper) and art_instance (e.g. bogus) may be needed
local wcls_input = {
    depos: wcls.input.depos(name=""),
};

// Collect all the wc/ls output converters for use below.  Note the
// "name" MUST match what is used in theh "outputers" parameter in the
// FHiCL that loads this file.
local wcls_output = {
    // ADC output from simulation
    sim_digits: wcls.output.digits(name="simdigits", tags=[sim_adc_frame_tag]),
    
    // The noise filtered "ADC" values.  These are truncated for
    // art::Event but left as floats for the WCT SP.  Note, the tag
    // "raw" is somewhat historical as the output is not equivalent to
    // "raw data".
    nf_digits: wcls.output.digits(name="nfdigits", tags=["raw"]),

    // The output of signal processing.  Note, there are two signal
    // sets each created with its own filter.  The "gauss" one is best
    // for charge reconstruction, the "wiener" is best for S/N
    // separation.  Both are used in downstream WC code.
    sp_signals: wcls.output.signals(name="spsignals", tags=["gauss", "wiener"]),

    // save "threshold" from normal decon for each channel noise
    // used in imaging
    sp_thresholds: wcls.output.thresholds(name="spthresholds", tags=["threshold"]),
};

local anode = tools.anodes[0];
local rng = tools.random; // BR insert
local drifter = sim.drifter;

// fill SimChannel
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



local signal = sim.signal;

local miscon = sim.misconfigure(params);

// Noise simulation adds to signal.
//local noise_model = sim.make_noise_model(anode, sim.empty_csdb);
local noise_model = sim.make_noise_model(anode, sim.miscfg_csdb);
local noise = sim.add_noise(noise_model);

local digitizer = sim.digitizer(anode, tag="orig");



local noise_epoch = "perfect";
//local noise_epoch = "after";
local chndb = chndb_maker(params, tools).wct(noise_epoch);
local nf = nf_maker(params, tools, chndb);

// signal processing
local sp = sp_maker(params, tools);


local sink = sim.frame_sink;

local graph = g.pipeline([wcls_input.depos,
                          drifter, 
                          wcls_simchannel_sink,
                          signal, miscon, noise, digitizer,
                          wcls_output.sim_digits,
                          nf,
                          wcls_output.nf_digits,
                          sp,
                          wcls_output.sp_signals,
                          sink]);

local graph2 = g.insert_node(graph, g.edge_labels("OmnibusSigProc", "FrameSplitter:sigsplitter"), wcls_output.sp_thresholds, wcls_output.sp_thresholds, name="graph2");


local app = {
    type: "Pgrapher",
    data: {
        edges: g.edges(graph2),
    },
};

// Finally, the configuration sequence which is emitted.

g.uses(graph2) + [app]
