#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in float texUnit;

uniform mat4 mvpMatrix;
out vec2 TexCoord;
out vec3 Normal;
out float TexUnit;

void main()
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
//	gl_Position /= gl_Position.w;
	TexCoord = texCoord;
	Normal = normal;
	TexUnit = texUnit;
}
