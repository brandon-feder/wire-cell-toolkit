local params = import "params/chooser.jsonnet";
local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";
{
    drifter: {
        type : "Drifter",
        data : {
            anode: wc.tn(anodes.nominal),
            DL : params.simulation.DL,
            DT : params.simulation.DT,
            lifetime : params.simulation.electron_lifetime,
            fluctuate : params.simulation.fluctuate,
        }
    },

    digitizer : {
        type: "Digitizer",
        data : {
            gain: params.simulation.digitizer.pregain,
            baselines: params.simulation.digitizer.baselines,
            resolution: params.simulation.digitizer.resolution,
            fullscale: params.simulation.digitizer.fullscale,
            anode: wc.tn(anodes.nominal),
        }
    },


}
