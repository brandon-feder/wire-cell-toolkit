// Main WCT configuration entry for MicroBooNE NS+SP (2D + L1) job run from art.

local wc = import "wirecell.jsonnet";

// Let user inject parameters from external sources (art/FHiCL, wire-cell CLI)
local par = import "params.jsonnet";


// Much of the configuration is partitioned to follow some (made up)
// conventions in order to allow sharing between different types of
// jobs (eg, just NF, just SP or NF+SP).  Each of the files providing
// the main "guts" must supply:
//
// edges - a list of edges for Pgrapher, if any
// 
// input - the edge head ({node:..., port:XXX}) for its single input
// 
// output - the edge tail for its single output
// 
// configs - an array of any configuration data structures to finally
// emit.  Only add to configs that which is explicitly defined in the
// file.  Service-like things (anode, chndb) need to be taken care of
// explicitly right here.
//
local gen = import "general.jsonnet";
local chndb = import "chndb.jsonnet";
local nf = import "nf.jsonnet";
local sp = import "sp.jsonnet";


// And, one special chunk that doesn't fully follow the above pattern
// which provides some intersticial nodes that do I/O with art::Event.
local wcls = import "wcls.jsonnet";


// We craft the data flow graph from the bits above.
local app = {
    type: "Pgrapher",
    data: {
        edges: [
            {
                tail: wcls.source,
                head: nf.input,
            }
        ] + nf.edges + [
            {
                tail: nf.output,
                head: wcls.nfsave
            },
            {
                tail: wcls.nfsave,
                head: sp.input,
            },
        ] + sp.edges + [
            {
                tail: sp.output,
                head: wcls.spsave,
            },
            {
                tail: wcls.spsave,
                head: wcls.sink,
            },
        ],
    }
};


// The final configuration list.  Note, order matters.
gen.configs + chndb.configs + nf.configs + sp.configs + wcls.configs + [app]
