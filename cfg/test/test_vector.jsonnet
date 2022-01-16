local v = import "vector.jsonnet";
local l1=[1,2,3];
local l2=[4,5,6];
local p={x:7, y:8, z:9};
{
    vadd:v.vadd(l1,l2),
    vmul:v.vmul(l1,l2),
    vsub:v.vsub(l1,l2),
    sum:v.sum(l1),
    mag:v.mag(l1),
    scale: v.scale(l1,2.0),
    topt: v.topoint(l1),
    frompt: v.frompoint(p),
}


