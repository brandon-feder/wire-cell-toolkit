local g = import "pgraph.jsonnet";

local a = {
    type: "A",
    data: {a:42},
};
local b1 = {
    type: "B",
    name: 'b1',
    data: {b:1},
    uses: [a],
};
local b2 = {
    type: "B",
    name: 'b2',
    data: {b:2},
    uses: [a],
};
local c = {
    type: "C",
    data: {c:2},
    uses: [b1, b2],
};
g.resolve_uses([c])

    
