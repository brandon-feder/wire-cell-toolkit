// collect all the filters into a list
local hf = import "hf_filters.jsonnet";
local lf = import "lf_filters.jsonnet";
[
    hf.gauss.tight,
    hf.gauss.wide,
    hf.weiner.tight.u,
    hf.weiner.tight.v,
    hf.weiner.tight.w,
    hf.weiner.wide.u,
    hf.weiner.wide.v,
    hf.weiner.wide.w,
    hf.wire.induction,
    hf.wire.collection,

    lf.roi.tight,
    lf.roi.tighter,
    lf.roi.loose,
]
