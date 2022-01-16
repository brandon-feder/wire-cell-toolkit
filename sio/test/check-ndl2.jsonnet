/* Demo the NumpyDepoLoader -> NumpySaver -> DepoBagger -> DepoFrameSink.

$ wire-cell \
  -A npzin=junk.npz \
  -A npzout=junk2.npz \
  -A tarout=junk2.tar.bz2 \
  -c sio/test/check-ndl2.jsonnet
*/

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

function(npzin="wct-depos.npz",
         npzout="wct-depos-copy.npz",
         tarout="wct-depos.tar.bz2")
{
    local graph = g.pipeline([

        g.pnode({
            type: 'NumpyDepoLoader',
            data: { filename: npzin }
        }, nin=0, nout=1),

        g.pnode({
            type: 'NumpyDepoSaver',
            data: { filename: npzout }
        }, nin=1, nout=1),

        g.pnode({
            type: 'DepoBagger'
        }, nin=1, nout=1),

        g.pnode({
            type: 'DepoFileSink',
            data: { outname: tarout },
        }, nin=1, nout=0)
    ]),

    local app = {
        type: 'TbbFlow',
        data: {
            edges: g.edges(graph)
        },
    },
    local plugins = [ "WireCellSio", "WireCellGen",
                      "WireCellApps", "WireCellPgraph", "WireCellTbb"],
    local cmdline = {
        type: "wire-cell",
        data: {
            plugins: plugins,
            apps: ["TbbFlow"],
        }
    },
    seq: [cmdline] + g.uses(graph) + [app],    
}.seq
