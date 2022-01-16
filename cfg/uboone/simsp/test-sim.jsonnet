local sim = import "sim.jsonnet";

local g = import "pgraph.jsonnet";
local dri = import "drifters.jsonnet";
local duc = import "ductors.jsonnet";
local dig = import "digitizers.jsonnet";
local noi = import "noise.jsonnet";



local test1 = [
    g.edge(dri.simple, duc.single),
    g.edge(duc.single, dig.simple),
    g.edge(dri.simple, duc.multi),
    g.edge(duc.multi, noi.nominal),
    g.edge(noi.nominal, dig.simple)
];

sim.single_noisy
