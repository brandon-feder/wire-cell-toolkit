// note: do not change this file for your particular job.  Rather
// inherit from these data structures and change them in your own
// file.
{
    source : {
        type: "MagnifySource",
        data: {
            filename: std.extVar("input"),
            frames: [],
            // Map between a channel mask map key name and its TTree for reading.
            cmmtree: [         
                //["bad", "T_bad"],
                //["lf_noisy", "T_lf"],
            ],
        }
    },
    celltreesource : {
        type: "CelltreeSource",
        data: {
            filename: std.extVar("input"),
            EventNo: std.extVar("evt"),
            frames: [],
        }
    },
    sink: {
        type: "MagnifySink",
        data: {
            input_filename: std.extVar("input"),
            output_filename: std.extVar("output"),

	    // this best be made consistent
	    anode: "AnodePlane",

            // The list of tags on traces to select groups of traces
            // to form into frames.
            frames: [],

            // The list of summary tags to save as 1D hisograms.
            summaries: [],

            // The evilness includes shunting data directly from input
            // file to output file.  This allows the shunt to be
            // limited to the listed histogram categories.  The
            // possible categories include: orig, raw, decon,
            // threshold, baseline and possibly others.  If the value
            // is left unset or null then all categories known to the
            // code will be shunted.
            shunt: [],

            // Map between a channel mask map key name and its TTree for writing.
            cmmtree: [         
                //["bad", "T_bad"],
                //["lf_noisy", "T_lf"],
            ],

        },
    }

}
