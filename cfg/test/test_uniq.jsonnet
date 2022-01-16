local wc = import "wirecell.jsonnet";
{
    l1 : [4,3,1,7,4],
    l2 : [1,4,9,0,0],
    "l1 unique" : wc.unique_list($.l1),
    "l1+l2 unique" : wc.unique_list($.l1 + $.l2),
}
