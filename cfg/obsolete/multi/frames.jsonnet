local params = import "params/chooser.jsonnet";
local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";

{
    defaults : {
        filename: std.extVar("framefile"), 
        units: if params.simulation.digitize then 1.0 else wc.uV,
        anode: wc.tn(anodes.nominal),
        readout_time: params.detector.readout_time,
    },
    histogram : {
        type: "HistFrameSink",
        data: $.defaults
    },
    celltree : {
        type: "CelltreeFrameSink",
        data: $.defaults,
    },
}
