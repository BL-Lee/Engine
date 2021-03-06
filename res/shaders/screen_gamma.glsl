#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D screenTexture;

void main()
{
  const float gamma = 2.2;
  vec3 hdrColor = texture(screenTexture, TexCoord).rgb;
  
  hdrColor = pow(hdrColor, vec3(1.0 / gamma));
   
  FragColor = vec4(hdrColor, 1.0);
}
