local wc = import "wirecell.jsonnet";

(import "cfg_wire_cell.jsonnet") + 
(import "cfg_trackdepos.jsonnet") +
(import "cfg_drifter.jsonnet") + 
(import "cfg_diffuser.jsonnet") +
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
		    head: wc.Node {type:"Diffuser", name:"diffuserU"}
		},
		{
		    tail: wc.Node {type:"Drifter", name:"drifterV"},
		    head: wc.Node {type:"Diffuser", name:"diffuserV"}
		},
		{
		    tail: wc.Node {type:"Drifter", name:"drifterW"},
		    head: wc.Node {type:"Diffuser", name:"diffuserW"}
		},

	    ]
	}
    },
]

