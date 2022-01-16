local params = import "params/chooser.jsonnet";
local wc = import "wirecell.jsonnet";
local anodes = import "multi/anodes.jsonnet";
{
    /// The noise model from empiric measurements
    empirical_model : {
        type: "EmpiricalNoiseModel",
        data: {
            anode: wc.tn(anodes.nominal),
            spectra_file: params.noise,
        }
    },
    /// And a noise source using this model
    empirical_source : {
        type: "NoiseSource",
        data: {
            model: wc.tn($.empirical_model),
            anode: $.empirical_model.data.anode,
        },
    },

    /// Package both into a list to make it easy to add into main.
    empirical: [$.empirical_model, $.empirical_source],

}
    
