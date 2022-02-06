#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texUnit;
layout(location = 4) in vec4 jointIndices;
layout(location = 5) in vec4 jointWeights;


uniform mat4 vpMatrix;
uniform mat4 mMatrix;
uniform mat4 normalMatrix;
uniform vec3 ViewPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 boneCompositeMatrices[64];

struct PointLight
{
  vec3 position;
  vec3 ambientColour;
  vec3 diffuseColour;
  vec3 specularColour;
  float linearFalloff;
};

//uniform DirectionalLight dLight;
#define POINT_LIGHT_COUNT 2
uniform PointLight pointLights[POINT_LIGHT_COUNT];

struct DirectionalLight
{
  vec3 direction;
  vec3 ambientColour;
  vec3 diffuseColour;
  vec3 specularColour;
};

//uniform DirectionalLight dLight;
#define DIRECTIONAL_LIGHT_COUNT 1
uniform DirectionalLight dirLights[DIRECTIONAL_LIGHT_COUNT];

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

uniform Material material;


vec3 calcPointLightContributionDiffSpec(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  //diffuse shading
  float diff = max(dot(lightDir, normal), 0.0);
  float lightDist = sqrt(dot(light.position - fragPos, light.position - fragPos));
  vec3 diffuse = (light.diffuseColour * diff);
  
  
  //specular
  float specularStrength = 1.0;
  vec3 reflectDir = reflect(-lightDir, normal);

  float specContribution = max(dot(viewDir, reflectDir), 0.0);
  float spec = pow(specContribution, 16);
  vec3 specular = specularStrength * spec * light.specularColour;

  diffuse *= material.diffuse;
  specular *= material.specular;
  
  float attenuation = 1 + light.linearFalloff * lightDist; //linear only at the moment  
  return ((specular + diffuse) / attenuation);
}
vec3 calcDirLightContributionDiffSpec(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  //diffuse shading
  float diff = max(dot(light.direction, normal), 0.0);
  vec3 diffuse = (light.diffuseColour * diff);
  
  //specular
  float specularStrength = 1.0;
  vec3 reflectDir = reflect(-light.direction, normal);

  float specContribution = max(dot(viewDir, reflectDir), 0.0);
  float spec = pow(specContribution, 16);
  vec3 specular = specularStrength * spec * light.specularColour;
  diffuse *= material.diffuse;
  //ambient *= material.ambient;
  specular *= material.specular;
  
  return specular + diffuse;
}


flat out vec3 ambient;
flat out vec3 diffSpec;
out vec4 lightSpaceCoords;
void main()
{
  vec3 diffSpecOut = vec3(0.0);
  
  vec4 newPos = vec4(0.0, 0.0, 0.0, 0.0);
  for (int i = 0; i < 4; i++)
    {
      newPos += boneCompositeMatrices[int(jointIndices[i])] * vec4(position,1.0) * jointWeights[i];
    }

  gl_Position = vpMatrix * mMatrix * newPos;
  vec3 WorldPos = vec3(mMatrix * newPos);
  vec3 Normal = normalize(mat3(normalMatrix) * normal);
  vec3 viewDir = normalize(ViewPos - WorldPos);
  vec3 ambientOut = vec3(0.0);
  for (int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
      diffSpecOut += calcPointLightContributionDiffSpec(pointLights[i], Normal, WorldPos, viewDir);
      ambientOut += (pointLights[i].ambientColour * material.ambient);
    }
  for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
      diffSpecOut += calcDirLightContributionDiffSpec(dirLights[i], Normal, WorldPos, viewDir);
      ambientOut += (dirLights[i].ambientColour * material.ambient);
    }
  diffSpec = diffSpecOut;
  ambient = ambientOut;

  lightSpaceCoords = lightSpaceMatrix * vec4(WorldPos, 1.0);
}
