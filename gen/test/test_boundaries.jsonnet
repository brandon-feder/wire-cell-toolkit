// This configuration tests the simulation boundaries by placing some
// short, ideal tracks at various specific points and times.

// These are provided by wire-cell-cfg pacakge which needs to be
// either in WIRECELL_PATH when this file is consumed by wire-cell or
// in a path given by "jsonnet -J <path> ..." if testing with that CLI.
local wc = import "wirecell.jsonnet";
local v = import "vector.jsonnet";

local cmdline = {
    type: "wire-cell",
    data: {
        plugins: ["WireCellGen", "WireCellPgraph", "WireCellSio"],
        apps: ["Pgrapher"]
    }
};

local random = {
    type: "Random",
    data: {
        generator: "default",
        seeds: [0,1,2,3,4],
    }
};
local utils = [cmdline, random];


// base detector parametrs.  Together, these match no real detector.
// The data structure is organized and named to have synergy with
// configuration objects for the simulation nodes defined later.
local base_params = {
    local par = self,           // make available to inner data structures

    lar : {
        DL :  7.2 * wc.cm2/wc.s,
        DT : 12.0 * wc.cm2/wc.s,
        lifetime : 8*wc.ms,
        drift_speed : 1.6*wc.mm/wc.us, // 500 V/cm
        density: 1.389*wc.g/wc.centimeter3,
        ar39activity: 1*wc.Bq/wc.kg,
    },
    detector : {
        // Relative extent for active region of LAr box.  
        // (x,y,z) = (drift distance, active height, active width)
        extent: [1*wc.m,1*wc.m,1*wc.m],
        // the center MUST be expressed in the same coordinate system
        // as the wire endpoints given in the files.wires entry below.
        // Default here is that the extent is centered on the origin
        // of the wire coordinate system.
        center: [0,0,0],
        drift_time: self.extent[0]/par.lar.drift_speed,
        drift_volume: self.extent[0]*self.extent[1]*self.extent[2],
        drift_mass: par.lar.density * self.drift_volume,
    },
    daq : {
        readout_time: 5*wc.ms,
        nreadouts: 1,
        start_time: 0.0*wc.s,
        stop_time: self.start_time + self.nreadouts*self.readout_time,
        tick: 0.5*wc.us,        // digitization time period
        sample_period: 0.5*wc.us, // sample time for any spectral data - usually same as tick
        first_frame_number: 100,
        ticks_per_readout: self.readout_time/self.tick,
    },
    adc : {
        gain: 1.0,
        baselines: [900*wc.millivolt,900*wc.millivolt,200*wc.millivolt],
        resolution: 12,
        fullscale: [0*wc.volt, 2.0*wc.volt],
    },
    elec : {
        gain : 14.0*wc.mV/wc.fC,
        shaping : 2.0*wc.us,
        postgain: -1.2,
    },
    sim : {
        fluctuate: true,
        digitize: true,
        noise: false,

        // continuous makes the WCT sim act like the streaming
        // detector+daq.  This means producing readout even if there
        // may be no depos.  If true then readout is based on chunking
        // up depos in time and there may be periods of time that do
        // not have any readouts.
        continuous: false,
    },
    files : {                   // each detector MUST fill these in.
        wires: null,
        fields: null,
        noise: null,
    }
};

local uboone_params = base_params {
    lar : super.lar {
        drift_speed : 1.114*wc.mm/wc.us, // at microboone voltage
    },
    detector : super.detector {
        // these two vectors define the origin of a Cartesian
        // coordinate system.  First sets the size of a box which is
        // the sensitive volume:
        extent: [2.5604*wc.m,2.325*wc.m,10.368*wc.m],
        // Next says where the center of that box is expressed
        // relative to whatever the origin is.  For MB we want this
        // box placed so that it has one plane at X=0 and another a
        // Z=0 and then centered on Y=0.
        center: [0.5*self.extent[0], 0.0, 0.5*self.extent[2]],
    },
    elec : super.elec {
        postgain: -1.2,
    },
    files : {
        wires:"microboone-celltree-wires-v2.1.json.bz2",
        fields:"ub-10-half.json.bz2",
        noise: "microboone-noise-spectra-v2.json.bz2",
    }
};


local dune_params = base_params {
    lar : super.lar {
        drift_speed : 1.6*wc.mm/wc.us, // 500 V/cm
    },
    detector : super.detector {
        extent : [3.594*wc.m, 5.9*wc.m, 2.2944*wc.m],
        center : [0, 0, 0],
    },  
    elec : super.elec {
        postgain: -1.0,
    },
    files : {
        wires:"dune-wires.json.bz2",
        fields:"garfield-1d-3planes-21wires-6impacts-dune-v1.json.bz2",
        // For now, pretend DUNE is post-noise-filtered microboone for
        // now.  This is bogus in that it gives its spectra for wire
        // lengths which are shorter so extraploation will not be so
        // accurate and may underestimate noise levels.
        noise: "microboone-noise-spectra-v2.json.bz2",
    },
};

local params = uboone_params;


// Here we make some ideal tracks to mark specific points in space and
// time.
local funcs = {
    ecks(t=0, l=1*wc.cm, c=v.topoint(params.detector.center), q=-5000) :: {
        ret : [
            {
                time: t,
                charge: q,
                ray: wc.ray(v.topoint(v.vshift(v.frompoint(c), [-1,0,-1], l)),
                            v.topoint(v.vshift(v.frompoint(c), [+1,0,+1], l))),
            },
            {
                time: t,
                charge: q,
                ray: wc.ray(v.topoint(v.vshift(v.frompoint(c), [-1,0,+1], l)),
                            v.topoint(v.vshift(v.frompoint(c), [+1,0,-1], l))),
            },
        ]
    }
};


//
// Make some crafty tracks to mark interesting places 
//

// A small shift to move a point from X=0 to the field response plane.
local vfrplane = [10*wc.cm, 0, 0];

// looking down on the detector, mark the four corner edges at the mid
// height of the detector.  "Upper" means the cathode.
local half_vext_equator = v.scale(v.vmul(params.detector.extent, [1,0,1]), 0.5);
local half_mext_equator = v.mag(half_vext_equator);
local cen_ll = v.vadd(vfrplane,
                      v.vshift(params.detector.center,
                               v.vmul(half_vext_equator, [-1, 0, -1]), half_mext_equator));
local cen_lr = v.vadd(vfrplane,
                      v.vshift(params.detector.center,
                               v.vmul(half_vext_equator, [-1, 0,  1]), half_mext_equator));
local cen_ul = v.vshift(params.detector.center,
                        v.vmul(half_vext_equator, [ 1, 0, -1]), half_mext_equator);
local cen_ur = v.vshift(params.detector.center,
                        v.vmul(half_vext_equator, [ 1, 0,  1]), half_mext_equator);

// Some time away from 0 where depositions begin
local t_start = 100*wc.ms;

// Time at which a depo at the CPA will arive to x=0 right at the end of a readout.
// Subtract off just a little epsilon so that something at t_end will be included in the depo group.
local t_end = t_start + params.daq.readout_time - params.detector.extent[0]/params.lar.drift_speed - 0.01*wc.ms;

local depos = {
    type: "TrackDepos",
    data: {
        step_size: 1.0 * wc.millimeter,
        group_time: if params.sim.continuous then -1 else params.daq.readout_time,
        tracks: funcs.ecks(t=t_start).ret
            +   funcs.ecks(c=v.topoint(cen_ll), t=t_start).ret
            +   funcs.ecks(c=v.topoint(cen_lr), t=t_start).ret
            +   funcs.ecks(c=v.topoint(cen_ul), t=t_start).ret
            +   funcs.ecks(c=v.topoint(cen_ur), t=t_start).ret
            +   funcs.ecks(c=v.topoint(cen_ul), t=t_end).ret
            +   funcs.ecks(c=v.topoint(cen_ur), t=t_end).ret
            +   funcs.ecks(c=v.topoint([10*wc.cm,0,half_vext_equator[2]]),
                           t= t_start).ret
            +   funcs.ecks(t=t_start+100*wc.ms).ret
    }
};

local wires = {
    type: "WireSchemaFile",
    data: { filename: params.files.wires }
};
local fields = {
    type: "FieldResponse",
    data: { filename: params.files.fields }
};
local anode = {
    type : "AnodePlane",        // 
    name : "nominal",
    data : params.elec + params.daq {
        ident : 0,
        field_response: wc.tn(fields),
        wire_schema: wc.tn(wires),
    }
};
local anodes = [wires, fields, anode];

local drifter = {
    type : "Drifter",
    data : params.lar + params.sim  {
        anode: wc.tn(anode),
    }
};

local ductor = {
    type : 'Ductor',
    name : 'nominal',
    data : params.daq + params.lar + params.sim {
        nsigma : 3,
	anode: wc.tn(anode),
    }
};
local signal = [drifter, ductor, depos];


local digitizer = {
    type: "Digitizer",
    data : params.adc {
        anode: wc.tn(anode),
    }
};

local numpy_saver = {
    data: params.daq {
        filename: "test-boundaries.npz",
        frame_tags: [""],       // untagged.
        scale: 1.0,             // ADC
    }
};
local numpy_depo_saver = numpy_saver { type: "NumpyDepoSaver" };
local numpy_frame_saver = numpy_saver { type: "NumpyFrameSaver" };


// not configurable, just name it.
local frame_sink = { type: "DumpFrames" };

local readout = [digitizer, numpy_depo_saver, numpy_frame_saver];

local graph_edges = [
    {
        tail: { node: wc.tn(depos) },
        head: { node: wc.tn(numpy_depo_saver) },
    },
    {
        tail: { node: wc.tn(numpy_depo_saver) },
        head: { node: wc.tn(drifter) },
    },
    {
        tail: { node: wc.tn(drifter) },
        head: { node: wc.tn(ductor) },
    },
    {
        tail: { node: wc.tn(ductor) },
        head: { node: wc.tn(digitizer) },
    },
    {
        tail: { node: wc.tn(digitizer) },
        head: { node: wc.tn(numpy_frame_saver) },
    },
    {                   // terminate the stream
        tail: { node: wc.tn(numpy_frame_saver) },
        head: { node: wc.tn(frame_sink) },
    },
];


// Here the nodes are joined into a graph for execution by the main
// app object.  
local app = {
    type: "Pgrapher",
    data: {
        edges: graph_edges,
        // debug: {
        //     tstart: t_start,
        //     tend: t_end,
        //     params: params,
        // }
    }
};

// Finally, we return the actual configuration sequence:
utils + anodes + signal + readout + [app]

