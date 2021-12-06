#version 330 core

//in vec2 TexCoord;
//flat in vec3 Normal;
//in vec3 WorldPos;
//flat in vec3 colour;
//out vec4 outColour;

uniform sampler2D tex;
uniform vec3 ViewPos;
/*
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


vec3 calcPointLightContribution(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  //diffuse shading
  float diff = max(dot(lightDir, normal), 0.0);
  float lightDist = sqrt(dot(light.position - fragPos, light.position - fragPos));
  vec3 diffuse = (light.diffuseColour * diff);
  
  //ambient
  vec3 ambient = (light.ambientColour * 0.5);
  
  //specular
  float specularStrength = 1.0;
  vec3 reflectDir = reflect(-lightDir, normal);

  float specContribution = max(dot(viewDir, reflectDir), 0.0);
  float spec = pow(specContribution, 16);
  vec3 specular = specularStrength * spec * light.specularColour;

  diffuse *= material.diffuse;
  ambient *= material.ambient;
  specular *= material.specular;
  
  float attenuation = 1 + light.linearFalloff * lightDist; //linear only at the moment  
  return ((ambient + specular + diffuse) / attenuation);
}
vec3 calcDirLightContribution(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  //diffuse shading
  float diff = max(dot(light.direction, normal), 0.0);
  vec3 diffuse = (light.diffuseColour * diff);
  
  //ambient
  vec3 ambient = (light.ambientColour * 0.5);
  
  //specular
  float specularStrength = 1.0;
  vec3 reflectDir = reflect(-light.direction, normal);

  float specContribution = max(dot(viewDir, reflectDir), 0.0);
  float spec = pow(specContribution, 16);
  vec3 specular = specularStrength * spec * light.specularColour;
  diffuse *= material.diffuse;
  ambient *= material.ambient;
  specular *= material.specular;
  
  return ambient + specular + diffuse;
}
*/
flat in vec3 colour;
out vec4 outColour;
void main()
{
  outColour = vec4(colour,1.0);
  /*
  vec3 outputColour = vec3(0.0);

  vec3 viewDir = normalize(ViewPos - WorldPos);
  
  for (int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
      outputColour += calcPointLightContribution(pointLights[i], Normal, WorldPos, viewDir);
    }
  for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
      outputColour += calcDirLightContribution(dirLights[i], Normal, WorldPos, viewDir);
    }
  
    outColour = vec4(outputColour,1.0);*/
}
