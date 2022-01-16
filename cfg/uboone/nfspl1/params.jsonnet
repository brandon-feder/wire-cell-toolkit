// This file provides an object of parameters used elsewhere in the
// configuration.

// The hard-coded defaults are patched, wholesale with a data
// structure provided via a std.extVar called "override".

// If users get pissy about having a big Jsonnet string passed in
// "override" then fine-grained, surgical additions of std.extVar()
// can be sprinkled throughout defaults.

local wc = import "wirecell.jsonnet";


local defaults = {

    // The art::Event label for the input raw::RawDigit collection
    raw_input_label: "daq",

    noisedb:{
        // This can be "wcls" (requires connection to Fermilab DB
        // servers) or "wct" (just static config).  It sets how the
        // misconfigured channels are found.
        flavor: "wcls",    

        // This can be "before" or "after" or "multi".  It sets how
        // the list of RMS cuts are found.
        epoch: "after",

        // A run number needed if flavor is multi.  It gives some run
        // between Run periods I [4952,6998] and II (>7956).  See
        // chndb.jsonnet.
        run12boundary: 7000, 
    },

    // The size of frequency domain spectra.  Input ADC waveforms will
    // be resized to match.  This should not be changed without
    // understanding the guts of NF+SP code and config data.
    frequency_bins: 9592,

    // Size for processed signal waveforms.  This number may be chosen
    // rather arbitarily.  The output waveforms will simply be end
    // truncated or zero-padded to match.
    output_nticks: 9595,

    // Larger configuration data files, which are searched for in WIRECELL_PATH.
    // Field response functions:
    fields_file: "ub-10-half.json.bz2",
    // File holding where wires are
    wires_file: "microboone-celltree-wires-v2.json.bz2",
    // Per channel responses
    chresp_file: "microboone-channel-responses-v1.json.bz2",

    // Digitizer sample period (aka "tick").  This should not be
    // changed without understanding all the code.
    sample_period: 0.5*wc.us,


};

// If testing with "jsonnet" CLI, must use --ext-code 'overide={...}' (not -V).
// Etc, if compiling with libjsonnet++.
local override = std.extVar("override");

std.mergePatch(defaults, override)
