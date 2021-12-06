#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in float TexUnit;
out vec4 outColour;

uniform sampler2D ambientTex;
uniform vec3 ambientColour;

void main()
{
  //vec3 ambient = dot(Normal, normalize(vec3(0.5,1.0,0.0))) * vec3(0.5,1.0,1.0);// ambientColour;
  outColour = texture(ambientTex, TexCoord) + vec4(TexUnit,0.0,0.0, 0.0);
}
