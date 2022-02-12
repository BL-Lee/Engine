#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texUnit;
layout(location = 5) in vec4 jointIndices;
layout(location = 4) in vec4 jointWeights;
uniform mat4 mMatrix;
uniform mat4 lightMatrix;

uniform mat4 boneCompositeMatrices[64];

void main()
{
  vec4 newPos = vec4(0.0, 0.0, 0.0, 0.0);
  for (int i = 0; i < 4; i++)
    {
      newPos += boneCompositeMatrices[int(jointIndices[i])] * vec4(position,1.0) * jointWeights[i];
    }
  gl_Position = lightMatrix * mMatrix * newPos;  
}
