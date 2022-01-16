local wc = import "wirecell.jsonnet"; // just for tn()

local attrs(a) =
    std.join(', ',['%s = "%s"' % [k,a[k]] for k in std.objectFields(a)]);


local edge(e, delim) =
    local a = if std.objectHas(e,'data') then e.data else {};
    '\t"%s" %s "%s"[%s];' %[e.tail.node, delim, e.head.node, attrs(a)];

local node(n) =
    local a = if std.objectHas(n,'data') then n.data else {};
    '\t"%s"[%s];' % [wc.tn(n), attrs(a)];


local object_split(obj,keys) = [
    {[k]:obj[k] for k in keys},
    std.prune(obj {[k]:null for k in keys}),
];
    

local graph(g) =
    local parts = object_split({ type:'digraph', name: 'G', attrs:{} } + g,
                               ['type','name','attrs']);
    std.join('\n', [
        "%s %s {" % [parts[0].type, parts[0].name] ] +
             ['\t%s = "%s";' % [k, parts[1][k]] for k in std.objectFields(parts[1])] +
             ["\tgraph[%s];" % attrs(parts[0].attrs)]);


local dot(g) =
    local parts = object_split({ type:'digraph', name: 'G', attrs:{} } + g.graph,
                               ['type','name','attrs']);
    local delim = {digraph: '->', graph: '--'}[parts[0].type];
    local lines = [
        ["%s %s {" % [parts[0].type, parts[0].name]],
        ['\t%s = "%s";' % [k, parts[1][k]] for k in std.objectFields(parts[1])],
        ["\tgraph[%s];" % attrs(parts[0].attrs)],
        [ node(n) for n in g.nodes ],
        [ edge(e,delim) for e in g.edges ],
        ['}'],
    ];
    std.join('\n',std.flattenArrays(lines));
                          
{
    osplit: object_split,

    node: node,
    edge: edge,
    graph: graph,
    dot: dot,
}
