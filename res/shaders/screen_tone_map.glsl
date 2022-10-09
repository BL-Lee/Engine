#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;
uniform int palettize;
uniform sampler3D blueNoise;
uniform sampler2D screenTexture;
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
  vec3 hdrColor = texture(screenTexture, TexCoord).rgb;
  
  //tone mapping
  hdrColor = ACESFilm(hdrColor);
  if (palettize != 0)
    {
      vec3 map = hdrColor.rgb;      
      FragColor = vec4(texture(blueNoise, map));
    }
  else
    {
      FragColor = vec4(hdrColor,1.0);
    }

  }
