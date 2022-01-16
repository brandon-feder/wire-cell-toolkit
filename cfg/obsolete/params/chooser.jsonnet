
local params_uboone = import "params/uboone.jsonnet";
local params_protodune = import "params/protodune.jsonnet";

local detector = std.extVar("detector");

if detector == "uboone" then params_uboone
else if detector == "protodune" then params_protodune
else {}

//{
//    params: if detector == "uboone" params_uboone else if detector == "protodune" then params_protodune else {},
//}
