local g = import "pgraph.jsonnet";
local f = import "pgrapher/common/funcs.jsonnet";

{
    local fpg = f.fanpipe([g.pnode({type:"Test",name:"test%d"%n, data:{number:n,index:n-1}}, nin=1, nout=1)
                           for n in std.range(1,6)], "testpipe", ["testtag"]),
    local fpga = { type: "Pgrapher", data: { edges: g.edges(fpg), }, },
    seq: g.uses(fpg) + [ fpga ], // make something looking like a config so dotify works

}.seq
