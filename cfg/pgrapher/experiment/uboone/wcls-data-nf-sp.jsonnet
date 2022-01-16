local seq = import "wcls-data-nf-sp-common.jsonnet";
local raw_input_label = std.extVar("raw_input_label"); // art:Event inputTag, eg "daq"
local epoch = std.extVar("epoch"); // eg "dynamic", "after", "before", "perfect"
seq(raw_input_label, epoch)
