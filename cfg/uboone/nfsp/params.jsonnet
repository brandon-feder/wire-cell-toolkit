// -*- js-mode -*-

// This file holds basic parameters used elsewhere.

// Relies on a few things from WIRECELL_PATH
local wc = import "wirecell.jsonnet";

{
    // The size of frequency domain spectra.  Input ADC waveforms will
    // be resized to match.
    frequency_bins: 9592,

    // Size for processed signal waveforms.  This number may be chosen
    // to match the same size as input ADC waveforms and so may differ
    // from frequency_bins.
    output_nticks: 9595,

    // Digitizer sample period (aka "tick")
    sample_period: 0.5*wc.us,


    // Data files.
    fields_file: "ub-10-half.json.bz2",
    wires_file: "microboone-celltree-wires-v2.json.bz2",
    chresp_file: "microboone-channel-responses-v1.json.bz2",
}
