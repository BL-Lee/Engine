#version 330 core

in vec2 TexCoord;
in vec3 Normal;
out vec4 outColour;

uniform sampler2D tex;

void main()
{
  vec3 outNormal = (Normal + 1.0) / 2.0;
  //outColour = texture(tex, TexCoord) * vec4(1.0, 1.0, 1.0, 1.0);
  outColour = vec4(outNormal, 1.0);
}
