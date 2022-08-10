#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texUnit;

uniform mat4 vpMatrix;
uniform mat4 mMatrix;
uniform mat4 normalMatrix;
uniform vec3 ViewPos;
uniform mat4 lightSpaceMatrix;

#include res/shaders/basicLightingIncludes.glsl

flat out vec3 ambient;
flat out vec3 diffSpecDir;
flat out vec3 diffSpecPoint;
out vec4 lightSpaceCoords;
void main()
{
  vec3 diffSpecOut = vec3(0.0);
  gl_Position = vpMatrix * mMatrix * vec4(position, 1.0);
  vec3 WorldPos = vec3(mMatrix * vec4(position, 1.0));
  //TexCoord = texCoord;
  //vec4 nextNormal = vpMatrix * mMatrix * vec4(normal,0.0);
  vec3 Normal = normalize(mat3(normalMatrix) * normal);
  vec3 viewDir = normalize(ViewPos - WorldPos);
  vec3 ambientOut = vec3(0.0);
  for (int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
      diffSpecOut += calcPointLightContributionDiffSpec(pointLights[i], Normal, WorldPos, viewDir);
      ambientOut += (pointLights[i].ambientColour * material.ambient);
    }
  diffSpecPoint = diffSpecOut;
  diffSpecOut = vec3(0.0);
  for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
      diffSpecOut += calcDirLightContributionDiffSpec(dirLights[i], Normal, WorldPos, viewDir);
      ambientOut += (dirLights[i].ambientColour * material.ambient);
    }
  diffSpecDir = diffSpecOut;
  ambient = ambientOut;

  lightSpaceCoords = lightSpaceMatrix * vec4(WorldPos, 1.0);
}
