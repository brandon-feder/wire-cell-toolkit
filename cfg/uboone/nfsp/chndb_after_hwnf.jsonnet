// The RMS cuts after the hardware noise fix.

// Here they get annoying as they the HW fix makes the noise so
// naturally low that a wire-length-dependent cut is needed using
// channel number as a proxy for wire length (on the U/V wires).

/*
After Hardware Fix
U-Plane     (i = channel number)
  Ch<600    = Cut(1+(0.7/600*i), 5)
  Ch>=600 - <1800 = Cut(1.7, 11)
  Ch>=1800 - <2400 = Cut(1+(0.7/600*(2399-i)), 5)

V-Plane
  Ch<600+2400  = Cut(0.8+(0.9/600*(i-2400)), 5)
  Ch>=600+2400 - <1800+2400 = Cut(1.7, 11)
  Ch>=1800+2400 - Ch<2400+2400 = Cut(0.8+(0.9/600*(4799-i)), 5)

Y-Plane
  Cut (1.25, 8) 

Note, the mid-channels for U/V have fixed cuts but below we
laboriously expand this in array comprehension anyways just to keep
each entry similar and avoid typo errors.

*/

local wc = import "wirecell.jsonnet";

[
    { channels: ch, min_rms_cut: 1 + ch*0.7/600, max_rms_cut:5.0}
     for ch in std.range(0,600-1)
] + [
    { channels: ch, min_rms_cut: 1.7, max_rms_cut:11.0}
     for ch in std.range(600,1800-1)
] + [
    { channels: ch, min_rms_cut: 1 + (2399-ch)*0.7/600, max_rms_cut:5.0}
     for ch in std.range(1800,2400-1)
] + [
    { channels: ch, min_rms_cut: 0.8 + (ch-2400)*0.9/600, max_rms_cut:5.0} 
     for ch in std.range(2400, 2400+600-1)
] + [
    { channels: ch, min_rms_cut: 1.7, max_rms_cut:11.0}                    
     for ch in std.range(2400+600, 2400+1800-1)
] + [
    { channels: ch, min_rms_cut: 0.8 + (4799-ch)*0.9/600, max_rms_cut:5.0} 
     for ch in std.range(2400+1800, 2400+2400-1)
] + [
    {
        channels: { wpid: wc.WirePlaneId(wc.Wlayer) },
        min_rms_cut: 1.25,
        max_rms_cut: 8.0,
    },

]

