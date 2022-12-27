/*
    static.frag
    by Spatial
    05 July 2013
*/

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}
//static.frag portion ends here

uint xorshift32(uint x)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

float rand(float seed)
{
  return floatConstruct(xorshift32(floatBitsToUint(seed)));
}
