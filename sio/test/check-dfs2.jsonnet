/* Demo the DepoFileSource -> DepoFrameSink.

$ wire-cell \
  -A tarin=in.tar.bz2 \
  -A tarout=out.tar.bz2 \
  -c sio/test/check-dfs2.jsonnet
*/

local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

function(tarin="wct-depos.tar.bz2",
         tarout="wct-depos-copy.tar.bz2")
{
    local graph = g.pipeline([

        g.pnode({
            type: 'DepoFileSource',
            data: { inname: tarin }
        }, nin=0, nout=1),

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
