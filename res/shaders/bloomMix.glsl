#version 330 core

//https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
uniform sampler2D srcTexture;
uniform sampler2D bloomTexture;
uniform float bloomStrength;
uniform float exposure;
in vec2 texCoord;
layout (location = 0) out vec3 FragColour;

//uniform int palettize;
//uniform sampler3D blueNoise;
vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp(((x*(a*x+b))/(x*(c*x+d)+e)), 0.0, 1.0);
}

void main()
{
  const float gamma = 2.0;
  vec3 hdrColour = texture(srcTexture, texCoord).rgb;

  vec3 bloomColour = texture(bloomTexture,texCoord).rgb;

  vec3 mixedColour = mix(hdrColour, bloomColour, bloomStrength);

  vec3 mapped = vec3(1.0) - exp(-mixedColour * exposure);
  
  //tone mapping
  mixedColour = ACESFilm(mapped);

  mixedColour = pow(mapped, vec3(1.0 / gamma));

  /*  if (palettize != 0)
    {
      vec3 map = mixedColour.rgb;      
      FragColour = texture(blueNoise, map).rgb;
    }
    else*/
    {
      FragColour = mixedColour;
    }
}

