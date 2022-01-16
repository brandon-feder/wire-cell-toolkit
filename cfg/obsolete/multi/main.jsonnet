/// This is a main configuration entry point which compiles to JSON
/// giving an array of component configuration objects.  See also
/// init.jsonnet, the compiled JSON from which should be prepended by
/// the application (ie:
///
///   wire-cell [...] -c multi/main.jsonnet
///

local params = import "params/chooser.jsonnet";
local init = import "multi/init.jsonnet";
local bits = import "multi/bits.jsonnet";
local anodes = import "multi/anodes.jsonnet";
local ductors = import "multi/ductors.jsonnet";
local depos = import "multi/depos.jsonnet";
local frames = import "multi/frames.jsonnet";
local noise = import "multi/noise.jsonnet";
local multiductor = import "multi/uboone.jsonnet"; // fixme: this needs if/then/else for when we support DUNE
local wc = import "wirecell.jsonnet";

// The list of configuration objects.
init + [

    depos.jsonfile,             // set up input file use "-V depofile=foo.json" on wire-cell CLI.

] + anodes.objects + [

    bits.drifter,               // one shared drifter
    
] + ductors.objects + noise.empirical + [

    multiductor,

    bits.digitizer,

    frames.celltree,

    {                           // the main "app" component
        type: "FourDee",
        data : {
            DepoSource: wc.tn(depos.jsonfile),
            Drifter: wc.tn(bits.drifter),
            Ductor: wc.tn(multiductor),
            Dissonance: wc.tn(noise.empirical[1]),

            Digitizer: wc.tn(bits.digitizer),
            
            FrameSink: wc.tn(frames.celltree),
        }
    },


]
