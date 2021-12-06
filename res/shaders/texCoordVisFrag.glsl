#version 330 core

in vec2 TexCoord;
in vec3 Normal;
out vec4 outColour;

uniform sampler2D tex;

void main()
{
  vec3 outNormal = vec3(TexCoord.x, 0.0, TexCoord.y);
  //outColour = texture(tex, TexCoord);
  outColour = vec4(outNormal, 1.0);
}
