local g = import "pgraph.jsonnet";

local obj = {
    a: [1,2],
    b: 42,
    c: {d:9},
};

{
    a: g.prune_key(obj, 'a'),
    b: g.prune_key(obj, 'b'),
    c: g.prune_key(obj, 'c'),
}
