#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in float texUnit;

out vec2 TexCoord;
out vec3 Normal;
out float TexUnit;
void main()
{
	gl_Position = vec4(position, 1.0);
	TexCoord = texCoord;
	Normal = normal;
	TexUnit = texUnit;
}
