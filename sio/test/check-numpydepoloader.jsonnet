// Demo the NumpyDepoLoader.
//
// $ wire-cell -A infile=junk.npz -c sio/test/test-numpydepoloader.jsonnet

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

function(infile="wct-depos.npz")
{
    local dl = g.pnode({
        type: 'NumpyDepoLoader',
        data: {
            filename: infile
        }
    }, nin=0, nout=1),

    local dd = g.pnode({
        type: 'DumpDepos',
    }, nin=1, nout=0),

    local graph = g.pipeline([dl, dd]),
    local app = {
        type: 'Pgrapher',
        data: {
            edges: g.edges(graph)
        },
    },
    local plugins = [ "WireCellSio", "WireCellGen",
                      "WireCellApps", "WireCellPgraph"],
    local cmdline = {
        type: "wire-cell",
        data: {
            plugins: plugins,
            apps: ["Pgrapher"],
        }
    },
    seq: [cmdline] + g.uses(graph) + [app],    
}.seq
