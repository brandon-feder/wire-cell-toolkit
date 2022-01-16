local g = import "pgraph.jsonnet";

{
    n1: g.pnode({type:"Node", name:"n1"}, nout=1),
    n2: g.pnode({type:"Node", name:"n2"}, nin=1, nout=1),
    n3: g.pnode({type:"Node", name:"n3"}, nin=1),
    n4: g.pnode({type:"Node", name:"n4"}, nin=1, nout=1),

    e12: g.edge(self.n1, self.n2),
    e23: g.edge(self.n2, self.n3),

    pn: g.intern([self.n1],[self.n3],[self.n2],[
        g.edge(self.n1, self.n2),
        g.edge(self.n2, self.n3)], "pn"),

    n12: g.intern([self.n1],[self.n2],edges=[
        g.edge(self.n1, self.n2)
    ], name="n12"),
    n123: g.intern([self.n12],[self.n3],edges=[
        g.edge(self.n12, self.n3),
    ], name="n123"),


    n13: g.intern([self.n1],[self.n3], edges=[
        g.edge(self.n1, self.n3),
    ], name="n13"),

    n123inserted: g.insert_one(self.n13, 0, self.n2, self.n2, name="n123inserted"),

    pipe: g.pipeline([self.n1,self.n2,self.n3]),

    ru: g.resolve_uses([]),


    pipe_uses: g.uses(self.pipe),
    pipe_edges: g.edges(self.pipe),

    pipe_indices12: g.find_indices(g.edges(self.pipe), self.e12),
    pipe_indices23: g.find_indices(g.edges(self.pipe), self.e23),
    pipe_indices13: g.find_indices(g.edges(self.pipe), g.edge(self.n1, self.n3)), // should be empty

    pn1243: g.insert_node(self.pipe, g.edge(self.n2, self.n3),
                         self.n4, self.n4, name="n1243"),
    pn1243_uses: g.uses(self.pn1243),
    pn1243_edges: g.edges(self.pn1243),

    // here we "cheat" by using out-of-band knowledge of node
    // type:name labels in order to form the edge to break without
    // having access to the fundamental pnodes that comprise the edge
    // endpoints.
    edge_to_break: g.edge_labels("Node:n2","Node:n3"),
    cheat: g.insert_node(self.pipe, self.edge_to_break,
                         self.n4, self.n4, name="n1243"),

    // note: this check is rather expensive as it recurses both data
    // structures.  one run of this file takes 0.2s w/out this line
    // and 4.1 s with.
    assert self.cheat == self.pn1243,

}

// Here is one way to turn this file into something that looks like a
// WCT configuration JSON file:
//
// jsonnet -J cfg cfg/test/test_pgraph.jsonnet |jq '.pn1243_uses + [{type:"Pgrapher",data:{edges:.pn1243_edges}}]'  > foo.json
//
// And make a pdf
// wirecell-pgraph dotify --jpath '-1' foo.json foo.pdf

