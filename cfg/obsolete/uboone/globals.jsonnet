/*
  This holds global parameters potentially shared by more than one component.
*/

local wc = import "wirecell.jsonnet";
{
    // must match what was used in Garfield field response calculation
    drift_speed: 1.114*wc.mm/wc.us,
    // FEE amplifier gain.  fixme: code needs to change to take this
    // quantity in the system of untis.  fixme: need to also set the
    // post-FEE gain of 1.2 somewhere.
    gain: 14.0*wc.mV/wc.fC,
    // FEE peaking time
    shaping: 2*wc.us,
    // RC constant 
    rc_constant: 1*wc.ms,
    // post-FEE gain
    postgain: 1.2,
    // How long to readout the detector at once.
    readout: 5.0*wc.ms,
    // sample period
    tick: 0.5*wc.us,  

    // from arXiv:1508.07059v2
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

    // Output ADC or Volts
    digitize: true,
    
}
