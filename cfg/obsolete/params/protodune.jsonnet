local common = import "params/common.jsonnet";
common {
    fields: {
        nominal: "garfield-1d-3planes-21wires-6impacts-dune-v1.json.bz2",
        truth: "garfield-1d-3planes-21wires-6impacts-dune-v1.json.bz2",
    },
    wires: "pdsp-wires.json.bz2",
    noise: "microboone-noise-spectra-v2.json.bz2",
}
