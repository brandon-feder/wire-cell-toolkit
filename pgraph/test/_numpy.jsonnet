local wc = import "wirecell.jsonnet";

local params = import "_params.jsonnet";


local base_saver = {
    data: params.daq {
        //filename: "uboone-wctsim.npz",
        filename: "uboone-wctsim-%(src)s-%(digi)s-%(noise)s.npz" % {
            src: "tracks",
            digi: if params.sim.digitize then "adc" else "volts",
            noise: if params.sim.noise then "noise" else "signal",
        },
        frame_tags: [""],       // untagged.
        scale: if params.sim.digitize then 1.0 else wc.uV,
    }
};

local depo_saver = base_saver { type: "NumpyDepoSaver" };
local frame_saver = base_saver { type: "NumpyFrameSaver" };

{
    depo: depo_saver,
    frame: frame_saver,

}
