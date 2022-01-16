// WARNING: the consumer of these filters is OmnibusSigProc and it
// hard codes the names of the filters.  So, it's important to match
// them exactly here.

local wc = import "wirecell.jsonnet";
{
    basic : {
        type: "LfFilter",
        data: {
	    max_freq: 1 * wc.megahertz,
	    tau: 0.0 * wc.megahertz,
        }
    },
    roi : {
        tight: $.basic {
            name: "ROI_tight_lf",
            data: super.data {
                tau: 0.02 * wc.megahertz,
            }
        },
        tighter: $.basic {
            name: "ROI_tighter_lf",
            data: super.data {
                tau: 0.1 * wc.megahertz,
            }
        },
        loose: $.basic {
            name: "ROI_loose_lf",
            data: super.data {
                tau: 0.0025 * wc.megahertz,
            }
        },

    }
}

