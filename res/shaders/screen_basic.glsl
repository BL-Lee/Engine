#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D screenTexture;

void main()
{
  vec3 hdrColor = texture(screenTexture, TexCoord).rgb;
  FragColor = vec4(hdrColor, 1.0);
}
