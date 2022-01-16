local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local s1 = g.pnode({
    type: "Source",
    name: "s1",
}, nout=1);
local s2 = g.pnode({
    type: "Source",
    name: "s2",
}, nout=1);

local j = g.pnode({
    type: "Join",
    name: "j",
}, nin=2);

[
    g.edge(s1,j,0,0),
    g.edge(s2,j,0,1),
]
