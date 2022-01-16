local wc = import "wirecell.jsonnet";
local g = import "pgraph.jsonnet";

local par = import "params.jsonnet";
local com = import "common.jsonnet";


{
    simple: g.pnode({
        type: "Digitizer",
        data: par.adc {
            anode: wc.tn(com.anode),
            frame_tag: "orig",  // this tag is checked by, eg, NF
        },
        uses: [com.anode],
    },nin=1,nout=1),
}
