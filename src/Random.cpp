
inline uint64_t xorshift64(uint64_t *state)
{
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return *state = x;
}
inline float randomUnilateral64(uint64_t *state)
{
  return (float)xorshift64(state) / (float)((uint64_t)-1);//4294967295.0f);
}
inline float randomBilateral64(uint64_t *state)
{
  return 1.0f - 2.0f*randomUnilateral64(state);
}
  
inline u32 xorshift32(u32 *state)
{
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  u32 x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}
inline f32 randomUnilateral32(u32 *state)
{
  return (f32)(xorshift32(state)) / ((uint32_t)-1);
}
inline f32 randomBilateral32(u32 *state)
{
  return 1.0f - 2.0f*randomUnilateral32(state);
}

