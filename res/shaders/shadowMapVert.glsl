#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 mMatrix;
uniform mat4 lightMatrix;

void main()
{
  gl_Position = lightMatrix * mMatrix * vec4(position, 1.0);  
}


