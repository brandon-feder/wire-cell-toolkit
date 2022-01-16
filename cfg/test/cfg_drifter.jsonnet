local wc = import "wirecell.jsonnet";
local Drifter = {type: "Drifter", name: "", data:{drift_velocity:wc.nominal_drift_velocity}};
[
    Drifter { name: "drifterU", data: Drifter.data {location: 15*wc.mm} },
    Drifter { name: "drifterV", data: Drifter.data {location: 10*wc.mm} },
    Drifter { name: "drifterW", data: Drifter.data {location:  5*wc.mm} },
]

