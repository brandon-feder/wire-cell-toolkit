local wc = import "wirecell.jsonnet";

(import "cfg_wire_cell.jsonnet") + 
(import "cfg_trackdepos.jsonnet") +
[
    {
	type:"TbbFlow",
	data: {
	    graph:[
		{
		    tail: wc.Node {type:"TrackDepos"},
		    head: wc.Node {type:"DumpDepos"}
		},
	    ]
	}
    },

]


