local g = import "pgraph.jsonnet";
local dot = import "dot.jsonnet";

local n1 = g.pnode({
    type:"Node", name:"n1",
    data:{
        color:'red'
    }}, nout=1);
local n2 = g.pnode({
    type:"Node", name:"n2",
    data: {
        color: "blue",
    }}, nin=1, nout=1);
local n3 = g.pnode({
    type:"Node", name:"n3",
    data: {
        color: "green"
    }}, nin=1);
local gr = g.intern([n1],[n3],[n2],[
    g.edge(n1, n2),
    g.edge(n2, n3)], "pn");

local graph = {
    graph: {
        name: "test",
        type: "digraph",
        rankdir: "LR",
        attrs: {
            label: "test graph",
            color: "red"
        },
    },
    edges: gr.edges,
    nodes: gr.uses,
};
dot.dot(graph)
//dot.graph(graph.graph)
//dot.osplit(graph.graph, ['name','type'])
