local wc = import "wirecell.jsonnet";

(import "cfg_wire_cell.jsonnet") + 
(import "cfg_trackdepos.jsonnet") +
(import "cfg_drifter.jsonnet") + 
[
    {
	type:"TbbFlow",
	data: {
	    graph:[
		{
		    tail: wc.Node {type:"TrackDepos"},
		    head: wc.Node {type:"Drifter", name:"drifterU"}
		},
		{
		    tail: wc.Node {type:"TrackDepos"},
		    head: wc.Node {type:"Drifter", name:"drifterV"}
		},
		{
		    tail: wc.Node {type:"TrackDepos"},
		    head: wc.Node {type:"Drifter", name:"drifterW"}
		},

		{
		    tail: wc.Node {type:"Drifter", name:"drifterU"},
		    head: wc.Node {type:"DumpDepos"}
		    //head: wc.Node {type:"DumpDepos", name:"ddU"}
		},
		{
		    tail: wc.Node {type:"Drifter", name:"drifterV"},
		    head: wc.Node {type:"DumpDepos"}
		    //head: wc.Node {type:"DumpDepos", name:"ddV"}
		},
		{
		    tail: wc.Node {type:"Drifter", name:"drifterW"},
		    head: wc.Node {type:"DumpDepos"}
		    //head: wc.Node {type:"DumpDepos", name:"ddW"}
		},

	    ]
	}
    },
    
]


