#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texUnit;

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
