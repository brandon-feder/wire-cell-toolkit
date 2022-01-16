// WARNING: the main consumer of these filters is OmnibusSigProc and
// it hard codes the names of the filters.  So, it's important to
// match them here.

local wc = import "wirecell.jsonnet";
{
    basic : {
        type: "HfFilter",
        data: {
	    max_freq: 1 * wc.megahertz,
	    sigma: 0.0 * wc.megahertz,
	    power: 2,
	    flag: true,
        },
    },

    gauss : {

        tight : $.basic {
            name: "Gaus_tight",
            data: super.data {
	        sigma: 1.11408e-01 * wc.megahertz,
            },
        },
        wide : $.basic {
            name: "Gaus_wide",
            data: super.data {
	        sigma: 0.14 * wc.megahertz,
            },
        },
    },

    weiner : {
        tight : {
            u : $.basic {
                name: "Wiener_tight_U",
                data: super.data {
                    sigma: 5.75416e+01/800.0*2 * wc.megahertz,
	            power: 4.10358e+00,
                }
            },
            v : $.basic {
                name: "Wiener_tight_V",
                data: super.data {
                    sigma: 5.99306e+01/800.0*2 * wc.megahertz,
	            power: 4.20820e+00,
                }
            },
            w : $.basic {
                name: "Wiener_tight_W",
                data: super.data {
                    sigma: 5.88802e+01/800.0*2 * wc.megahertz,
	            power: 4.17455e+00,
                }
            }
        },
        wide :{
            u : $.basic {
                name: "Wiener_wide_U",
                data: super.data {
                    sigma: 1.78695e+01/200.0*2 * wc.megahertz,
	            power: 5.33129e+00,
                }
            },
            v : $.basic {
                name: "Wiener_wide_V",
                data: super.data {
                    sigma: 1.84666e+01/200.0*2 * wc.megahertz,
	            power: 5.60489e+00,
                }
            },
            w : $.basic {
                name: "Wiener_wide_W",
                data: super.data {
                    sigma: 1.83044e+01/200.0*2 * wc.megahertz,
	            power: 5.44945e+00,
                }
            },
        },
    },

    wire: {
        basic_data: {
            max_freq: 1, // warning: units
            power: 2,
            flag: false,
        },

        induction: $.basic {
            type: "HfFilter",
            name: "Wire_ind",
            data: $.wire.basic_data {
	        sigma: 1.0/std.sqrt(3.1415926)*1.4,
            }
        },
        collection: $.basic {
            type: "HfFilter",
            name: "Wire_col",
            data: $.wire.basic_data {
                sigma: 1.0/std.sqrt(3.1415926)*3.0,
            }
        },
    },
}

