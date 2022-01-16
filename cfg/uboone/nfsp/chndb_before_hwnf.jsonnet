// The RMS cuts before the hardware noise fix
/*
Before HF
U-Plane  
  Ch<100   =  Cut (1, 5)
  Ch>=100 <2000 = Cut(1.9, 11)
  Ch>=2000 <2400 = Cut(0.9, 5)

V-plane
  Ch<290+2400 = Cut(1, 5)
  Ch>=290+2400 <2200+2400 = Cut(1.9, 11)
  Ch>=2200+2400 <2400+2400 = Cut(1, 5)
Y-Plane 
  Cut(1.25, 8)

*/

local wc = import "wirecell.jsonnet";

[
    // min/max RMS cut
    {
        channels: std.range(0,99) + std.range(2400, 2400+289),
        min_rms_cut: 1.0,
        max_rms_cut: 5.0,
    },
    {
        channels: std.range(100,1999) + std.range(2400+290, 2400+2199),
        min_rms_cut: 1.9,
        max_rms_cut: 11.0,
    },
    {
        channels: std.range(2000, 2399),
        min_rms_cut: 0.9,
        max_rms_cut: 5.0,
    },
    {
        channels: std.range(2400+2200, 2400+2399),
        min_rms_cut: 1.0,
        max_rms_cut: 5.0,
    },
    {
        channels: { wpid: wc.WirePlaneId(wc.Wlayer) },
        min_rms_cut: 1.25,
        max_rms_cut: 8.0,
    },

]
