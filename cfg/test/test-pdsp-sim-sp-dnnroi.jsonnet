// This provides a main wire-cell config file to exercise
// sim+sigproc+dnnroi.  When run it will produce tar files of frames
// data as numpy arrays.  Ionization pattern is from ideal line
// source.

local wc = import "wirecell.jsonnet";
local pg = import "pgraph.jsonnet";
local params = import "pgrapher/experiment/pdsp/simparams.jsonnet";
local hs = import "pgrapher/common/helpers.jsonnet";

local wires = hs.aux.wires(params.files.wires);
local anodes = hs.aux.anodes(wires, params.det.volumes);

// simulation

// kinematics: ideal line source
local tracklist = [
   {
       time: 0,
       charge: -5000,         
       ray: params.det.bounds,
   },
];
local depos = pg.pipeline([
    hs.gen.track_depos(tracklist),
    hs.gen.bagger(params.daq),
]);

local random = hs.gen.random();
local drifter = hs.gen.drifter(params.det.volumes,params.lar,random);

// responses
local sim_fr = hs.aux.fr(params.files.fields[0]);
local er = hs.aux.cer(params.elec.shaping, params.elec.gain,
                      params.elec.postgain,
                      params.daq.nticks, params.daq.tick);
local rc = hs.aux.rc(1.0*wc.ms, params.daq.nticks, params.daq.tick);
local pirs = hs.gen.pirs(sim_fr, [er], [rc]);

// sp fr may differ from sim fr (as it does from real fr)
local sp_fr = hs.aux.fr(if std.length(params.files.fields)>1
                        then params.files.fields[1]
                        else params.files.fields[0]);
                        
local sp_filters = import "pgrapher/experiment/pdsp/sp-filters.jsonnet";
local adcpermv = hs.utils.adcpermv(params.adc);
local chndbf = import "pgrapher/experiment/pdsp/ocndb-perfect.jsonnet";
local chndb(anode) = chndbf(anode, sp_fr, params.nf.nsamples);
local dnnroi_override = {
    sparse: true,
    use_roi_debug_mode: true,
    use_multi_plane_protection: true,
    process_planes: [0, 1, 2]
};

local ts = {
    type: "TorchService",
    name: "dnnroi",
    data: {
        model: "unet-l23-cosmic500-e50.ts",
        device: "cpu",
        concurrency: 1,
    },
};

// little function to return a frame file tap or sink (if cap is
// true).  This bakes in PDSP-specific array bounds!
local out(anode, prefix, tag_pats, digitize=false, cap=false) = 
    local tags = [tp + std.toString(anode.data.ident)
                  for tp in tag_pats];
    local fname = prefix + "-"
                  + std.join("-", tags) + ".tar.bz2";
    local dense = hs.io.frame_bounds(2560, 6000,
                                     2560 * anode.data.ident);
    if cap
    then hs.io.frame_file_sink(fname, tags, digitize, dense)
    else hs.io.frame_file_tap(fname, tags, digitize, dense);
                         

local anode_pipeline(anode, prefix) = pg.pipeline([
    // sim
    hs.gen.signal(anode, pirs, params.daq, params.lar, rnd=random),
    hs.gen.noise(anode, params.files.noise, params.daq, rnd=random),
    hs.gen.digi(anode, params.adc),
    out(anode, prefix, ["orig"], true),
        
    // nf+sp
    hs.nf(anode, sp_fr, chndb(anode), params.nf.nsamples, params.daq.tick),
    hs.sp(anode, sp_fr, er, sp_filters, adcpermv,
          override=dnnroi_override),
    out(anode, prefix, ["wiener","gauss"]),

    // // dnnroi
    hs.dnnroi(anode, ts, output_scale=1.2),
    out(anode, prefix, ["dnnsp"], cap=true),
]);

function(prefix="test-pdsp-ssd") 
    local pipes = [ anode_pipeline(a, prefix) for a in anodes];
    local body = pg.fan.fanout('DepoSetFanout', pipes);
    local graph = pg.pipeline([depos, drifter, body]);
    hs.utils.main(graph, 'TbbFlow', ['WireCellPytorch'])

    

