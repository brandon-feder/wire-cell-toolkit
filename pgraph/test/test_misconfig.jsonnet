// This configs WCT simulation including misconfiguring of certain channels.

// These are provided by wire-cell-cfg pacakge which needs to be
// either in WIRECELL_PATH when this file is consumed by wire-cell or
// in a path given by "jsonnet -J <path> ..." if testing with that CLI.
local wc = import "wirecell.jsonnet";

local utils = import "_utils.jsonnet";
local anode = import "_anode.jsonnet";

local saver = import "_numpy.jsonnet";

local kine = import "_kine.jsonnet";
local noise = import "_noise.jsonnet";
local signal = import "_signal.jsonnet";
local miscfg = import "_misconfig.jsonnet";
local sink = import "_sink.jsonnet";


local edges = kine.edges + [
    {
        tail: kine.output,
        head: {node: wc.tn(saver.depo)},
    },
    {
        tail: {node: wc.tn(saver.depo)},
        head: signal.input,
    }
] +signal.edges + [
    {
        tail: signal.output,
        head: miscfg.input,
    }
] +miscfg.edges + [
    {
        tail: miscfg.output,
        head: noise.input,
    }
] +noise.edges + [
    {
        tail: noise.output,
        head: {node: wc.tn(sink.digitizer)},
    },
    {
        tail: { node: wc.tn(sink.digitizer) },
        head: { node: wc.tn(saver.frame) },
    },
    {                   // terminate the stream
        tail: { node: wc.tn(saver.frame) },
        head: { node: wc.tn(sink.sink) },
    },
];


// Here the nodes are joined into a graph for execution by the main
// app object.  
local app = {
    type: "Pgrapher",
    data: {
        edges: edges,
    }
};

local extra = [saver.depo, saver.frame, sink.digitizer, sink.sink, app];

utils.cfgseq + anode.cfgseq + kine.cfgseq + signal.cfgseq + miscfg.cfgseq + noise.cfgseq + extra





