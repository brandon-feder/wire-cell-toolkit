#ifndef WIRECELL_BITS
#define WIRECELL_BITS

namespace WireCell {
    namespace Bits {
        // Shift the value to the right for "n" bits and replace the first "n" bits by "filling"
        int shift_right(int value, int n, int filling, int totalBit);
        // Get the lowest "n" bits ...
        int lowest_bits(int value, int n);
    }  // namespace Bits
}  // namespace WireCell

#endif
