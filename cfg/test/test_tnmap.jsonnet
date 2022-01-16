local wc = import "wirecell.jsonnet";

local a1 = { type:'Foo', name: 'foo1' };
local a2 = { type:'Foo', name: 'foo2' };
local b = {type:'Bar'};
{
    seq1: [a1,a2,b],
    seq2: [a1,b],

    sequ: wc.unique_list($.seq1 + $.seq2)

    // cfgs1: std.map(function(x) wc.tn(x), $.seq1),
    // cfgs2: std.map(function(x) wc.tn(x), $.seq2),

    // tnlu: wc.tnmap($.seq1 + $.seq2),

    // cfgs: wc.unique_list($.cfgs1 + $.cfgs2),


    // cfgseq: [$.tnlu[x] for x in $.cfgs],



//    xxx: std.mapWithKey(, $.tnmap1)
}
