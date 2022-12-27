#version 330 core

uniform sampler2D shadowMap;
uniform sampler3D blueNoise;
uniform bool palettize;

#define PCF_SHADOWS 1
#define SHADOWS 1
#define POISSON_SHADOWS_STRATIFIED 1

//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

flat in vec3 ambient;
flat in vec3 diffSpecDir;
flat in vec3 diffSpecPoint;
in vec4 lightSpaceCoords;
out vec4 outColour;

#include res/shaders/bitUtils.glsl
#include res/shaders/shadowAndToneMappingUtils.glsl

void main()
{

#if SHADOWS
  // perform perspective divide
  vec3 projCoords = lightSpaceCoords.xyz / lightSpaceCoords.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  float bias = 0.005;
  float shadow = 0.0;

#if PCF_SHADOWS
  shadow += shadowValuePCF(projCoords, bias);  
#else //PCF_SHADOWS
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
#endif //PCF_SHADOWS
  
  outColour = vec4(ambient + diffSpecPoint + (1.0 - shadow) * diffSpecDir, 1.0);
#else //SHADOWS
  outColour = vec4(ambient +  diffSpecPoint + diffSpecDir, 1.0);
#endif //SHADOWS
}

