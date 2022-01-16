
#include "WireCellUtil/Bits.h"

int WireCell::Bits::shift_right(int value, int n, int filling, int totalBit)
{
    return (value >> n) | (filling << (totalBit - n));
}

int WireCell::Bits::lowest_bits(int value, int n) { return (value & ((1 << n) - 1)); }
