#version 330 core

uniform sampler2D shadowMap;
uniform sampler2D blueNoise;


#define PCF_SHADOWS 1
#define SHADOWS 1

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

flat in vec3 ambient;
flat in vec3 diffSpec;
in vec4 lightSpaceCoords;
out vec4 outColour;
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

void main()
{
  
#if SHADOWS
  // perform perspective divide
  vec3 projCoords = lightSpaceCoords.xyz / lightSpaceCoords.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  float bias = 0.005;

#if PCF_SHADOWS
  //Blend
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
        {
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
  shadow /= 9.0;

#else //PCF_SHADOWS
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
#endif //PCF_SHADOWS
  outColour = vec4(ambient + (1.0 - shadow) * diffSpec, 1.0);
#else //SHADOWS
  outColour = vec4(ambient +  diffSpec, 1.0);
#endif //SHADOWS
}

