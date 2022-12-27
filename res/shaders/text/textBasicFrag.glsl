#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in float TexUnit;
out vec4 outColour;

uniform sampler2D tex[16];

void main()
{
  outColour = texture(tex[int(TexUnit)], TexCoord);
  //outColour = vec4(int(TexUnit), int(TexUnit), 1.0, 1.0);
}



