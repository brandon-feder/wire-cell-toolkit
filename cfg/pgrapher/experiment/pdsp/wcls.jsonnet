// This collects some common patterns for configuring WCT to run
// inside LArSoft ("WC/LS").  It returns a function that returns
// "tools" that can finally be used in a top/higher level file.


local g = import 'pgraph.jsonnet';
local wc = import 'wirecell.jsonnet';

function(params, tools) {

    // Configure a raw frame source.  The raw_input_label is the name
    // of the RawDigits in the art::Event.  The "name" must match what
    // is set in the "inputers" list in the WCLS_Tool FHiCL config.
    adc_source(name, raw_input_label, tags=['orig']) :: g.pnode({
        type: 'wclsRawFrameSource',
        name: name,
        data: {
            art_tag: raw_input_label,
            frame_tags: tags,
            nticks: params.daq.nticks,
        },
    }, nin=0, nout=1),

    // A "mega" anode configuration.  The "name" must appear also in
    // the "outputers" list in the WCLS_Tool FHiCL config.  This is
    // likely the "anode" that should be provided to the frame_saver()
    // functions when operating on multi-APA detectors.
    mega_anode(name='meganodes') :: {
        type: 'MegaAnodePlane',
        name: name,
        data: {
            anodes_tn: [wc.tn(anode) for anode in tools.anodes],
        },
    },


    // Save a frame back to the art::Event.  The "name" must appear
    // also in the "outputers" list.  

    frame_saver(anode, name, tags=[], cmms=[]) :: g.pnode({
        type: 'wclsFrameSaver',
        name: name,
        data: {
            anode: wc.tn(anode),
            digitize: true,
            frame_tags: tags,
            nticks: params.daq.nticks,
            chanmaskmaps: cmms,
        },
    }, nin=1, nout=1, uses=[anode]),

    // Specialize defaults for noise filtered output
    nf_saver(anode, name='nfsaver', tags=['raw'], cmms=['bad']) ::
    $.frame_saver(anode, name,tags,cmms),

    // Specialize defaults for signal processed output
    sp_saver(anode, name='spsaver', tags=['gauss','wiener'], cmms=[]) ::
    $.frame_saver(anode, name, tags, cmms),

}
