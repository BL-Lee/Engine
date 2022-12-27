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

struct LightingPartials
{
  vec3 pointDiffuseAndSpecular;
  vec3 ambient;
  vec3 directionalDiffuseAndSpecular;
};
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

LightingPartials calcLightingPartials(vec3 worldPos, vec3 viewDir, vec3 normal)
{
  vec3 ambientOut = vec3(0.0);
  LightingPartials partials = LightingPartials(vec3(0.0), vec3(0.0), vec3(0.0));
  for (int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
      partials.pointDiffuseAndSpecular += calcPointLightContributionDiffSpec(pointLights[i], normal, worldPos, viewDir);
      partials.ambient += (pointLights[i].ambientColour * material.ambient);
    }
  for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
      partials.directionalDiffuseAndSpecular += calcDirLightContributionDiffSpec(dirLights[i], normal, worldPos, viewDir);
      partials.ambient += (dirLights[i].ambientColour * material.ambient);
    }
  return partials;
}
