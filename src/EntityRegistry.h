#ifndef ENTITY_REGISTRY_HEADER
#define ENTITY_REGISTRY_HEADER

struct Entity
{
  u32 id;
  char name[256];
  vec3 position;
  vec3 scale;
  vec3 rotation;
  vec3 velocity;
  vec3 angularVelocity;
  Mesh** meshes;
  char* vertShader;
  char* fragShader;
  u32 meshCount;
  bool visible;
};

#define MAX_REGISTRY_SIZE 2048

struct EntityRegistry
{
  Entity entities[MAX_REGISTRY_SIZE];
  u8 occupiedIndices[MAX_REGISTRY_SIZE];
  u32 entityCount;
  u32 entityCounter;
};

static EntityRegistry* globalEntityRegistry;



#endif
