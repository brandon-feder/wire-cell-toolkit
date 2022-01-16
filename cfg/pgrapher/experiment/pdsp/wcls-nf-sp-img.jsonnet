// configure a Wire-Cell/larsoft job for nf+sp+img

// Meta parameters to be set in the WCLS_Tool FHiCL configuration.
// 
// Set to "data" for data else simulation related parameters are used
local reality = std.extVar('reality');
//
//  The art::Event label or tag for the raw waveform input, eg "daq"
local raw_input_label = std.extVar('raw_input_label');


local wc = import 'wirecell.jsonnet';
local g = import 'pgraph.jsonnet';
local f = import "pgrapher/common/funcs.jsonnet";


local data_params = import 'params.jsonnet';
local simu_params = import 'simparams.jsonnet';
local params = if reality == 'data' then data_params else simu_params;

local tools_maker = import 'pgrapher/common/tools.jsonnet';
local tools = tools_maker(params);

local wcls_maker = import 'wcls.jsonnet';
local wcls = wcls_maker(params, tools);
local sp_maker = import 'sp.jsonnet';
local sp = sp_maker(params, tools, { sparse: true} );
local nf_maker = import 'nf.jsonnet';
local chndb_maker = import 'chndb.jsonnet';
local chndb = chndb_maker(params, tools);
local img = import "pgrapher/experiment/pdsp/img.jsonnet";


// two-faced components.
local meganodes = wcls.mega_anode();
local source = wcls.adc_source("adcs", raw_input_label);
local nf_saver = wcls.nf_saver(meganodes, 'nfsaver');
local sp_saver = wcls.sp_saver(meganodes, 'spsaver');

local nanodes = std.length(tools.anodes);
local anode_idents = [a.data.ident for a in tools.anodes];
local anode_indices = [n for n in std.range(0, nanodes-1)];

// These rules get applied for the raw frame fanout and must match what the chsels expect.
local fanout_tagrules = [{frame: {'orig': 'orig%d'%ind}} for ind in anode_indices];

// We will fan out a full frame and then put one channel selector per
// APA to output a narrowed frame.  This begins the per-APA pipelines.
local chsels = [
    g.pnode({
        type: 'ChannelSelector',
        name: 'chsel-' + tools.anodes[ind].name,
        data: {
            channels: std.range(2560 * ind, 2560 * (ind + 1) - 1),
            tags: ['orig%d' % ind],
        },
    }, nin=1, nout=1) for ind in anode_indices ];

// Fan out per-APA frames by 2 so we can fan them back in to sink them
// to art::Event while continuing each per-APA frame on to do imaging.
local ffo = [g.pnode({
    type:'FrameFanout',
    name:'ffo-'+anode.name,
    data:{
        multiplicity:2,
        tags: [],
    }}, nin=1, nout=2) for anode in tools.anodes];
local frame_fanin = g.pnode({
    type: 'FrameFanin',
    name: 'sigffi',
    data: {
        multiplicity: nanodes, 
        tags: [],
    }}, nin=nanodes, nout=1);

local frame_sink = g.pnode({ type: 'DumpFrames' }, nin=1, nout=0);

local tap = g.intern(innodes=ffo, centernodes=[sp_saver, frame_fanin], outnodes=[frame_sink],
                     edges = [g.edge(ffo[ind], frame_fanin, 1, ind) for ind in anode_indices]
                     + [g.edge(frame_fanin, sp_saver), g.edge(sp_saver, frame_sink)]);


// This builds the mainline per-APA pipes (chsel + nf + sp + imging parts).
local make_a_pipe = function(ind) {
    local anode = tools.anodes[ind],
    local aname = anode.name,
    ret : g.pipeline([
        chsels[ind],
        nf_maker(params, tools.anodes[ind], chndb.perfect(anode), ind, name='nf-'+aname),
        sp.make_sigproc(anode, 'sigproc-'+aname),
        ffo[ind],
        img.slicing(anode, aname),
        img.tiling(anode, aname),
        img.solving(anode, aname),
        img.dump(anode, aname, params.lar.drift_speed),
    ], "nf-sp-img-" + aname)
}.ret;
local perapa_pipes = [ make_a_pipe(ind) for ind in anode_indices];
local snspimg = f.fansink('FrameFanout', perapa_pipes, "snspimg", fanout_tagrules);
local mainline = g.pipeline([source, snspimg]);

local app = {
  type: 'Pgrapher',
  data: {
      edges: g.edges(mainline) + g.edges(tap),
  },
};

// Finally, the configuration sequence
g.uses(mainline) + g.uses(tap) + [app]



