#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;

uniform mat4 vpMatrix;
out vec3 Colour;
void main()
{
	gl_Position = vpMatrix * vec4(position, 1.0);
	Colour = colour;
}
