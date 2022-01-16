local wc = import "wirecell.jsonnet";
{
    foo : { type: "Foo" },
    foo_tn : wc.tn($.foo),
    bar : { type: "Bar", name: "bar" },
    bar_tn : wc.tn($.bar),
    baz : { type: "Baz", name: "" },
    baz_tn : wc.tn($.baz),
}

