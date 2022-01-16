// This is a WCT configuration file for use in a WC/LS data job.
//
// There are converter components that MUST be named in the FHiCL's
// "inputer" and "outputer" parameters and here.  Their type/name MUST
// match exactly.  In cases where a single instance of a type is
// needed the name is empty.  Besides having to match, these names are
// not particularly meaningful and so we "hard-code" their
// FHiCL/Jsonnet correspondance.
//
// On the other hand, some parameters must be coordinated which are
// subject to user modification.  These MUST be provided in the FHiCL
// in the "params" object.  These parameters are:


// Wrap this monster in a function so that we can switch between which
// channel noise database object we use.  The "epoch" variable is
// "dynamic" means the full wclsMultiChannelNoiseDB object is used.
// Otherwise  the it may be "before", "after" or "perfect".
function(raw_input_label, epoch = "dynamic") {

    local wc = import "wirecell.jsonnet",
    local g = import "pgraph.jsonnet",


    local params = import "pgrapher/experiment/uboone/params.jsonnet",
    local tools_maker = import "pgrapher/common/tools.jsonnet",
    local tools = tools_maker(params),
    local sim_maker = import "pgrapher/experiment/uboone/sim.jsonnet",
    local sim = sim_maker(params, tools),

    local wcls_maker = import "pgrapher/ui/wcls/nodes.jsonnet",
    local wcls = wcls_maker(params, tools),

    // for dumping numpy array for debugging
    local io = import "pgrapher/common/fileio.jsonnet",

    local nf_maker = import "pgrapher/experiment/uboone/nf.jsonnet",
    local chndb_maker = import "pgrapher/experiment/uboone/chndb.jsonnet",

    local sp_maker = import "pgrapher/experiment/uboone/sp.jsonnet",

    
    // This tags the output frame of the WCT simulation and is used in a
    // couple places so define it once.
    local sim_adc_frame_tag = "orig",

    local chndbm = chndb_maker(params, tools),

    local chndb = if epoch == "dynamic" then chndbm.wcls_multi(name="")
    else chndbm.wct(epoch),


    // Collect the WC/LS input converters for use below.  Make sure the
    // "name" argument matches what is used in the FHiCL that loads this file.
    local wcls_input = {
        adc_digits: g.pnode({
            type: 'wclsRawFrameSource',
            name: "",
            data: {
                art_tag: raw_input_label,
                frame_tags: ["orig"],
                nticks: params.daq.nticks,
            },
        }, nin=0, nout=1),

    },

    // Collect all the wc/ls output converters for use below.  Note the
    // "name" MUST match what is used in theh "outputers" parameter in the
    // FHiCL that loads this file.
    local wcls_output = {
        // The noise filtered "ADC" values.  These are truncated for
        // art::Event but left as floats for the WCT SP.  Note, the tag
        // "raw" is somewhat historical as the output is not equivalent to
        // "raw data".
        nf_digits: g.pnode({
            type: "wclsFrameSaver",
            name: "nfsaver", 
            data: {
                anode: wc.tn(tools.anode),
                digitize: true,         // true means save as RawDigit, else recob::Wire
                frame_tags: ["raw"],
                nticks: params.daq.nticks,
                chanmaskmaps: ["bad"],
            },
        }, nin=1, nout=1, uses=[tools.anode]),


        // The output of signal processing.  Note, there are two signal
        // sets each created with its own filter.  The "gauss" one is best
        // for charge reconstruction, the "wiener" is best for S/N
        // separation.  Both are used in downstream WC code.
        sp_signals: g.pnode({
            type: "wclsFrameSaver",
            name: "spsaver", 
            data: {
                anode: wc.tn(tools.anode),
                digitize: false,         // true means save as RawDigit, else recob::Wire
                frame_tags: ["gauss"],
                nticks: params.daq.nticks,
                chanmaskmaps: [],
            },
        },nin=1, nout=1, uses=[tools.anode]),
    },


    local anode = tools.anodes[0],

    local nf = nf_maker(params, tools, chndb),

    // signal processing
    local sp = sp_maker(params, tools),


    local sink = sim.frame_sink,

    local graph = g.pipeline([wcls_input.adc_digits,
                              nf,
                              wcls_output.nf_digits,
                              sp,
                              wcls_output.sp_signals,
                              sink]),


    local app = {
        type: "Pgrapher",
        data: {
            edges: g.edges(graph),
        },
    },

    // Finally, the configuration sequence which is emitted.

    seq: g.uses(graph) + [app]
}.seq

