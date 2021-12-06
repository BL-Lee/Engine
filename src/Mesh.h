#ifndef __MESH_HEADER
#define __MESH_HEADER


#include "Collision.h"
struct Vertex
{
  vec3 pos;
  vec3 normal;
  vec2 texCoord;
  float texUnit;
};
struct RendererMeshData
{
  u32 vertexBufferKey;
  u32 indexBufferKey;
  u32 indexCount;
  u32 vertexArrayKey;
  u32 shaderProgramKey;
  u32 ambientTextureKey;
  u32 diffuseTextureKey;
};

struct Material
{
  char* name;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  f32 specularExponent;
  f32 opticalDensity;
  f32 dissolve;
  s32 illumination;
  char* ambientMap;
  char* diffuseMap;
  char* specularTextureMap;
  char* specularHighlightMap;
  char* alphaMap;
  char* bumpMap;
};

struct Mesh
{
  bool visible;
  u32 vertexCount;
  Vertex* vertices;
  u32* indices;
  RendererMeshData rendererData;
  Material material;
  Collider collider;
};

#endif
