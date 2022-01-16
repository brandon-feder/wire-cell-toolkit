// This WCT configuration file provides all the setup needed for
// running "Omnibus" noise filter (NF) and signal processing (SP)
// chaing except for configuring the Omnibus app compontent itself.
//


local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet"; 
local magnify = import "uboone/io/magnify.jsonnet";
local omni = import "uboone/sigproc/omni.jsonnet";
local bits = import "uboone/sigproc/bits.jsonnet";
local spfilters = import "uboone/sigproc/filters.jsonnet";


{
    // this will probably be needed elsehwere for consistentcy
    anode: anodes.nominal,

    noisedb: omni.noisedb,

    misc_components : [
	// anode plane
	anodes.nominal,

	// channel noise database
	$.noisedb,

	// field
	bits.fieldresponse,

	// per channel response
	bits.perchanresp,

    ],

    noise_filters : [
	// individual noise filters used by the main filter.
	omni.channel_filters.bitshift,
	omni.channel_filters.single,
	omni.channel_filters.grouped,
	omni.channel_filters.status,
    ],

    sigproc_filters: spfilters,
    
    noise_frame_filters : [
	// The main noise frame filter
	omni.noisefilter,
	// The PMT noise frame filter
	omni.pmtfilter,
    ],
    sigproc_frame_filters : [
	// the main signal processing frame filter
	omni.sigproc,
    ],

    // separate NF and SP list above in case caller wants to insert
    // stuff between (like MagnifySink).  If caller just wants a
    // monolith then they can use:
    frame_filters: $.noise_frame_filters + $.sigproc_frame_filters,

    // gather all the stuff from here that needs to be in the main
    // configuration sequence *except* omnibus itself.  
    config_sequence : $.misc_components + $.noise_filters + $.sigproc_filters + $.frame_filters,

    // An example omnibus configuration.  Likely this needs to be
    // reproduced by the file that uses this file in irder to insert
    // any different filters and to supply source and sink.
    omnibus : {
        type: "Omnibus",
        data: {
            //source: wc.tn(source),
            //sink: wc.tn(sink),
            filters: std.map(wc.tn, $.frame_filters),
        }
    },
    
}
