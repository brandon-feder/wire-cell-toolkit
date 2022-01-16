local wc = import "wirecell.jsonnet";
{
    // Define TPC detector.
    detector : {

	// field response file
	field: null,		// must provide

	// wire geometry and numbers
	wires: null,		// must provide

	// FEE amplifier gain. 
	gain: 14.0*wc.mV/wc.fC,

	// FEE peaking time
	shaping: 2*wc.us,

	// post-FEE gain
	postgain: 1.2,

	// sample period
	tick: 0.5*wc.us,  

	// nsamples in a frame
	nticks: 10000,

	// How long to readout the detector at once.  Note, redefining
	// simulation.nticks in derived data recalculates this value.
	readout_time: $.detector.nticks * $.detector.tick,

    },

    // Parameters for simulation
    simulation: {

	// Data modeling the detector noise.
	noise: null, 		// must provide

        // must match what was used in Garfield field response calculation
	drift_speed: 1.6*wc.mm/wc.us,

	// Diffusion constants from arXiv:1508.07059v2
	DL:  7.2 * wc.cm2/wc.s,
	DT: 12.0 * wc.cm2/wc.s,

	// read off RHS of figure 6 in MICROBOONE-NOTE-1003-PUB
	electron_lifetime: 8*wc.ms,

	// A free choice: number of sigma before truncating a diffusion
	nsigma_diffusion_truncation : 3.0,

	// True if simulation should do fluctuations
	fluctuate: false,
	
	// Starting time of the simulation
	start_time: 0*wc.s,

	// The "event" number
	start_frame_number: 100,

	// Output ADC?  Else Volts
	digitize: true,

	// Digitizer related parameters
	digitizer : {
            // relative gain at the input of the digitizer
            pregain: 1.0,
            // plane baselines at input of digitizer
            baselines: [900*wc.millivolt,900*wc.millivolt,200*wc.millivolt],
            // resolution in bits
            resolution: 12,
            // full scale range
            fullscale: [0*wc.volt, 2.0*wc.volt],
	},

    },

    sigproc: {
	// The time-domain sample period assumed when constructing
	// frequency-domain spectra for noise filtering.
	sample_period: 0.5*wc.us,

	// The number of frequency bins in a frequency-domain spectra
	// for noise filtering.  Ideally this matches the number of
	// time domain samples (nticks) but this may cause a mismatch
	// in actual input data.
	frequency_bins: $.detector.nticks,

    },




}
