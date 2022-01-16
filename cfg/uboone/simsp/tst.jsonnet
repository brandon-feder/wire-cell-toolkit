local wc = import "wirecell.jsonnet";
local duc = import "ductors.jsonnet";

// local popuses(l, obj) = if std.objectHas(obj, "uses")
// then l + std.foldl(popuses, obj.uses, []) + [std.prune(std.mapWithKey(function (k,v) if k == "uses" then null else v, obj))]
// else l + [obj];
    
// local resolve_uses(seq) = wc.unique_list(std.foldl(popuses, seq, []));

// resolve_uses(duc.ductors)

wc.resolve_uses(duc.ductors)
