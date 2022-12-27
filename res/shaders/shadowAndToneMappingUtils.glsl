vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
                             );
  
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp(((x*(a*x+b))/(x*(c*x+d)+e)), 0.0, 1.0);
}

float shadowValuePCF(vec3 projCoords, float bias)
{
  float shadow = 0.0;
  //Blend   
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  int minWidth = -1;
  int maxWidth = 1;
  float contribution = 1.0 / ((maxWidth - minWidth + 1) * (maxWidth - minWidth + 1)) ;
  for(int x = minWidth; x <= maxWidth; ++x)
    {
      for(int y = minWidth; y <= maxWidth; ++y)
        {
       
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          shadow += projCoords.z - bias > pcfDepth ? contribution : 0.0;        
        }    
    }

  /*
    {
    for (int i = 0; i < 4; ++i)
    {
    float seed = projCoords.x + projCoords.y;
    uint index = xorshift32(floatBitsToUint(seed)) % 4u;
    float depth = texture(shadowMap, projCoords.xy + poissonDisk[index] / 700.0).r;
    shadow += currentDepth - bias > depth ? 0.2 : 0.0;
        
    }
    }

    shadow /= 2.0;*/
  return shadow;
}
