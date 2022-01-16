// Demo the NumpyDepoSaver.
//
// $ wire-cell -A outfile=junk.npz -c sio/test/test-numpydeposaver.jsonnet

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

function(outfile)
{

    local tracklist = [
        {
            time: 0*wc.ms,
            charge: -5000,
            ray: {
                tail: wc.point(t*5+10, -(2*t), -(2*t + 0.1), wc.cm),
                head: wc.point(t*5   , +(2*t), +(2*t + 0.1), wc.cm),
            }
        } for t in [1,2,3,4,5,6,7]],

    local depos = g.pnode({
        type: 'TrackDepos',
        data: {
            step_size: 1.0*wc.mm,
            tracks: tracklist
        },
    }, nin=0, nout=1),

    local ds = g.pnode({
        type: 'NumpyDepoSaver',
        data: {
            filename: outfile
        }
    }, nin=1, nout=1),

    local dd = g.pnode({
        type: 'DumpDepos',
    }, nin=1, nout=0),

    local graph = g.pipeline([depos, ds, dd]),
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
