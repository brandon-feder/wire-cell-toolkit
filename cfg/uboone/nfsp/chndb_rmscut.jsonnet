local before_hwnf = import "chndb_before_hwnf.jsonnet";
local after_hwnf = import "chndb_after_hwnf.jsonnet";

local hwnf_epoch = std.extVar("hwnf_epoch");

if hwnf_epoch == "before" then before_hwnf
else if hwnf_epoch == "after" then after_hwnf
else []
